//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_NETWORKLAYER_H
#define PROTOCOL_NETWORKLAYER_H

#ifndef DEBUG
extern "C" {
#include <FreeRTOS.h>
#include <semphr.h>
#include <portmacro.h>
#include <projdefs.h>
};
#endif

#include <++FreeRTOS.h>
#include "DataStructs.h"
#include "useful.h"
#include "StackInterfaces.h"
#include "Commands.h"
#include "SystemCommands.h"

RegisterCommand_t RegisterCommand;
ACKCommand_t ACKCommand;
Ping_t Ping;

//using namespace FreeRTOS;

class NetworkLayer : public NETInterface {
private:
    StackError stackState_;
    StackError updateStackState(StackError now) {
        if( now != STACK_OK ) {
            stackState_ = now;
            return stackState_;
        }
        return now;
    }

    CommandsTable table;
    Command * descriptorTable[256];
    DataDescriptor ** dataTable;
    uint8_t dataTableLen;

    FreeRTOS::Semaphore ackSemaphore;

public:

    virtual StackError stackState() {
        return stackState_;
    }

    NetworkLayer(PHYInterface * phy, DataDescriptor ** dataTable, uint8_t len) : table(descriptorTable, 255) {
        stackState_ = STACK_NO_CONNECTION;
        ackSemaphore.give();
        registerLowerLayer(phy);
        phy->registerUpperLayer(this);
        this->dataTable = dataTable;
        this->dataTableLen = len;
        for(uint8_t i = 0; i < len; ++i) {
            uint8_t id = dataTable[i]->id;
            printf("assigning i = %d, id = %d\n",i, id);
            descriptorTable[id] = dataTable[i];
            descriptorTable[id]->net = this;
        }
        printf("all data registered!\n");
        RegisterCommand.net  = this;
        descriptorTable[RegisterCommand.id] = &RegisterCommand;
        ACKCommand.net = this;
        descriptorTable[ACKCommand.id] = &ACKCommand;
        Ping.net = this;
        descriptorTable[Ping.id] = &Ping;
    }

    void testConnection() {
        stackState_ = Ping.send();
        printf("ping: %d\n", stackState());
    }

    virtual StackError passDownWithACK(const NETDataStruct & data) {
        for (uint8_t i = 0; i < 10; ++i) {
            printf("transmitting and waiting for ack\n");
            ackSemaphore.take(0);

            StackError res = this->passDown(data);
            if( res != STACK_OK ) {
                stackState_ = res;
                return res;
            }
            if ( ackSemaphore.take(100) ) {
                printf("ACK got!\n");
                return STACK_OK;
            } else {
                printf("PACKET LOSS!!!!\n");
            }
        }
        return updateStackState(STACK_TIMEOUT);
    }

    virtual StackError passDown(const NETDataStruct & data) {
        printf("[network] sending (id = %d): ", data.command);
        print_byte_table(data.data, data.len);

        PHYDataStruct phyData;
        phyData.append(data.command);
        phyData.append(data.data, data.len);

        return updateStackState(phyInterface->passDown(phyData));
    }

    virtual StackError passUp(const PHYDataStruct & data) {
//        NETDataStruct netData(data.data[0], data.data+1, data.len-1);
        uint8_t id = data.data[0];
        printf("[network] received (id = %d): ", id);
        print_byte_table(data.data+1, data.len-1);
        dataTypeUnion val;
        memcpy(val.bytes, data.data+1, data.len-1);
        if( descriptorTable[id]->ack ) {
            ACKCommand.send();
        }
        descriptorTable[id]->passUp(val);
    }

    virtual void receivedACK() {
        ackSemaphore.give();
    }

    virtual StackError sendSubscriptions() {
        for(uint8_t i = 0; i < dataTableLen; ++i) {
            RegisterCommand.send(*dataTable[i]);
        }
    }
};


#endif //PROTOCOL_NETWORKLAYER_H

