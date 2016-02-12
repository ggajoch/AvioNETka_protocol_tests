#ifndef PROTOCOL_SYSTEMCOMMANDS_H
#define PROTOCOL_SYSTEMCOMMANDS_H


#include "Commands.h"

class RegisterCommand_t : public Command {
public:
    explicit RegisterCommand_t() : Command(255, true) {
    }
    void send(const DataDescriptor & descriptor) {
        NETDataStruct data(this->id);
        data.append(descriptor.id);
        data.append(descriptor.fsxId);
        data.append(descriptor.encode());
        data.append(descriptor.ack);
        this->passDown(data);
    }
protected:
    virtual void callback(const dataTypeUnion &data) {
    }
};

class ACKCommand_t : public Command {
public:
    explicit ACKCommand_t() : Command(250, false) {
    }
    void send() {
        NETDataStruct data(this->id);
        this->passDown(data);
    }
protected:
    virtual void callback(const dataTypeUnion &data) {
    }
};



#endif //PROTOCOL_SYSTEMCOMMANDS_H
