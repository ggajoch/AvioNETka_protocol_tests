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

#include "DataStructs.h"
#include "useful.h"
#include "StackInterfaces.h"
#include "Commands.h"
#include "SystemCommands.h"

RegisterCommand_t RegisterCommand;
ACKCommand_t ACKCommand;

class NetworkLayer : public NETInterface {
    CommandsTable table;
    Command * descriptorTable[256];

#ifndef DEBUG
    SemaphoreHandle_t ACKSemaphore;
#endif
public:
    NetworkLayer(PHYInterface & phy, DataDescriptor ** dataTable, uint8_t len) : table(descriptorTable, 255) {
        registerLowerLayer(&phy);
#ifndef DEBUG
        ACKSemaphore = xSemaphoreCreateBinary();
#endif
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

        for(uint8_t i = 0; i < len; ++i) {
            RegisterCommand.send(*dataTable[i]);
        }
    }


    virtual void passDownWithACK(const NETDataStruct & data) const {
#ifndef DEBUG
        for(uint8_t i = 0; i < 3; ++i) {
            printf("transmitting and waiting for ack\n");
            xSemaphoreTake(ACKSemaphore, 0);

            this->passDown(data);
            bool res = (bool) xSemaphoreTake(ACKSemaphore, 1000);
            res &= 0x1;
            if( res ) {
                printf("ACK got!\n");
                break;
            } else {
                printf("PACKET LOSS!!!!\n");
            }
        }
#else
        this->passDown(data);
#endif
    }

    virtual void passDown(const NETDataStruct & data) const {
        printf("[network] sending (id = %d): ", data.command);
        print_byte_table(data.data, data.len);

        PHYDataStruct phyData;
        phyData.append(data.command);
        phyData.append(data.data, data.len);

        phyInterface->passDown(phyData);
    }

    virtual void passUp(const PHYDataStruct & data) const {
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
};


#endif //PROTOCOL_NETWORKLAYER_H
