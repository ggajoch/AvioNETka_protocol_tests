#ifndef PROTOCOL_SYSTEMCOMMANDS_H
#define PROTOCOL_SYSTEMCOMMANDS_H


#include "Commands.h"

class RegisterCommand_t : public Command {
public:
    explicit RegisterCommand_t() : Command(255, true) {
    }
    StackError send(const DataDescriptor & descriptor) {
        NETDataStruct data(this->id);
        data.append(descriptor.id);
        data.append(descriptor.fsxId);
        data.append(descriptor.encode());
        data.append(descriptor.ack);
        return this->passDown(data);
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
        //printf("[CMD] received ACK!");
        this->net->receivedACK();
    }
};


class Ping_t : public Command {
    FreeRTOS::Semaphore semaphore;
public:
    explicit Ping_t() : Command(251, false) {
        semaphore.give();
    }
    StackError send() {
        //printf("Testing PING...\n");
        NETDataStruct data(this->id);
        data.append((uint8_t)1);
        semaphore.take(0);
        StackError res = this->passDown(data);
        if( res != STACK_OK ) {
            return res;
        }
        if( semaphore.take(1000) ) {
            return STACK_OK;
        } else {
            return STACK_TIMEOUT;
        }
    }
protected:
    virtual void callback(const dataTypeUnion &data) {
        if(data.bytes[0] == 1) {
            //printf("Answering to PING!\n");
            NETDataStruct dataout(this->id);
            dataout.append((uint8_t)2);
            this->passDown(dataout);
        } else if(data.bytes[0] == 2) {
            semaphore.give();
            //printf("Got PONG!\n");
        }
    }
};


class SendSubscriptions_t : public Command {
public:
    explicit SendSubscriptions_t() : Command(252, false) {
    }
    void send() {
    }
protected:
    virtual void callback(const dataTypeUnion &data) {
        net->markSubscriptions();
    }
};


#endif //PROTOCOL_SYSTEMCOMMANDS_H
