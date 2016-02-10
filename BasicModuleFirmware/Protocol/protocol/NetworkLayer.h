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

#include "StackInterfaces.h"
#include "DataStructs.h"
#include "useful.h"
#include "ProtocolPackets.h"


class NetworkLayer : public NETInterface {
    PHYInterface * phyInterface;
    PresentationInterface * fsxInterface;
    const DataDescriptorsTable * descriptors;
#ifndef DEBUG
    SemaphoreHandle_t ACKSemaphore;
#endif
    uint8_t registered_commands;
public:
    NetworkLayer() : registered_commands(0), descriptors(0) {
#ifndef DEBUG
        ACKSemaphore = xSemaphoreCreateBinary();
#endif
    }

    void registerLowerLayer(PHYInterface * phyInterface) {
        this->phyInterface = phyInterface;
    }

    void registerUpperLayer(PresentationInterface * fsxInterface) {
        this->fsxInterface = fsxInterface;
    }

    void sendWithACK(const PHYDataStruct & data) {
#ifndef DEBUG
        for(uint8_t i = 0; i < 3; ++i) {
            printf("transmitting and waiting for ack\n");
            xSemaphoreTake(ACKSemaphore, 0);

            phyInterface->passDown(data);
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
        phyInterface->passDown(data);
#endif
    }

    virtual void passDown(const NETDataStruct & data) {
        printf("[network] sending (id = %d): ", data.command);
        print_byte_table(data.data, data.len);

        PHYDataStruct phyData;
        phyData.append(data.command);
        phyData.append(data.data, data.len);
        phyInterface->passDown(phyData);

        if( this->descriptors->at(data.command).ack ) {
            sendWithACK(phyData);
        } else {
            phyInterface->passDown(phyData);
        }
    }

    virtual void passDownRegistration(const NETDataStruct & data) {
        printf("[network] sending  registration (id = %d): ", data.command);
        print_byte_table(data.data, data.len);
        PHYDataStruct phyData;
        phyData.append(data.command);
        phyData.append(registered_commands);
        phyData.append(data.data, data.len);
        phyData.append(descriptors->at(registered_commands).ack);

        registered_commands++;
        sendWithACK(phyData);
    }


    virtual void passUp(const PHYDataStruct & data) {
        NETDataStruct netData(data.data[0], data.data+1, data.len-1);
        printf("[network] received (id = %d): ", netData.command);
        print_byte_table(netData.data, netData.len);
        if(netData.command == ACK_ID ) {
            printf("got ACK packet!\n");
#ifndef DEBUG
            xSemaphoreGive(ACKSemaphore);
#endif
        }
        if( this->descriptors->at(netData.command).ack ) {
            printf("sending ACK!\n");
            NETDataStruct val = makeACKPacket();
            this->passDown(val);
        }
        fsxInterface->passUp(netData);
    }

    virtual void registerDataDescriptors(const DataDescriptorsTable * descriptors) {
        this->descriptors = descriptors;
    }
};


#endif //PROTOCOL_NETWORKLAYER_H
