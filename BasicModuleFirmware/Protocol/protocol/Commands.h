#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include "DataStructs.h"
#include "StackInterfaces.h"

class Command {
protected:
    virtual void callback(const dataTypeUnion & data) = 0;
    Command(uint8_t id, bool ack) : id (id), ack(ack) {
    }
public:
    uint8_t id;
    bool ack;
    NETInterface * net;

    static uint8_t commands_registered;
    static void reset() {
        commands_registered = 0;
    }

    Command(bool ack) : ack(ack) {
        printf("registering command with id = %d\n", commands_registered);
        this->id = commands_registered;
        commands_registered++;
    }


    void passUp(const dataTypeUnion &  data) {
        if( ack ) {
            // TODO: send ACK
        }
        this->callback(data);
    }
    void passDown(const NETDataStruct & data) {
        if( ack ) {
            net->passDownWithACK(data);
        } else {
            net->passDown(data);
        }
    }
};

uint8_t Command::commands_registered;


class DataDescriptor : public Command {
public:
    uint32_t fsxId;
    bool rxEnabled = true;

    DataDescriptor(uint32_t FSXid, bool ack) : Command(ack) {
        this->fsxId = FSXid;
        this->ack = ack;
    };

    virtual void callback(const dataTypeUnion & data) = 0;

    virtual uint8_t encode() const = 0;
};

template<typename type>
class TypedDataDescriptor : public DataDescriptor {
    void (*callbackFunction)(type);
public:
    static const uint8_t len = sizeof(type);
    static const uint8_t value;

    TypedDataDescriptor(uint32_t FSXid, bool ack) : DataDescriptor(FSXid, ack) {}

    TypedDataDescriptor(uint32_t FSXid, bool ack, void (*callback)(type)) : DataDescriptor(FSXid, ack) {
        callbackFunction = callback;
    };

    virtual void callback(const dataTypeUnion & data);

    dataTypeUnion pack(type value);

    void send(type value) {
        NETDataStruct data(this->id);
        dataTypeUnion val = this->pack(value);
        data.append(val.bytes, this->len);
        this->passDown(data);
    }

    virtual uint8_t encode() const {
        return this->value;
    }
};

#define GEN(type, asType, EncodeValue) \
template<> \
void TypedDataDescriptor<type>::callback(const dataTypeUnion & data) { \
    this->callbackFunction(data.asType); \
} \
template<> \
dataTypeUnion TypedDataDescriptor<type>::pack(type value) { \
    dataTypeUnion anUnion; \
    anUnion.asType = value; \
    return anUnion;\
}\
template<> \
const uint8_t TypedDataDescriptor<type>::value = EncodeValue; \

GEN(bool, asBool, 0);
GEN(float, asFloat, 1);
GEN(uint8_t, asUint8, 2);
GEN(uint16_t, asUint16, 3);
GEN(uint32_t, asUint32, 4);
GEN(int8_t, asInt8, 5);
GEN(int16_t, asInt16, 6);
GEN(int32_t, asInt32, 7);


class CommandsTable {
    Command **table;
public:
    uint8_t len;

    CommandsTable() : table(0), len(0) { }

    CommandsTable(Command **table, uint8_t len) : table(table), len(len) { }

    Command &at(const uint8_t index) {
        return (*table[index]);
    }

    Command &operator[](const uint8_t index) {
        return (*table[index]);
    }
};

#endif //COMMANDS_H

