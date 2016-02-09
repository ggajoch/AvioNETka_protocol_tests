//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_NETWORKLAYER_H
#define PROTOCOL_NETWORKLAYER_H

extern "C" {
#include <FreeRTOS.h>
#include <semphr.h>
#include <portmacro.h>
#include <projdefs.h>
};

#include <event_groups.h>
#include "PhysicalLayer.h"
#include "ProtocolPackets.h"


class NetworkLayer : public NETInterface {
    PHYInterface * phyInterface;
    FSXInterface * fsxInterface;
    SemaphoreHandle_t ACKSemaphore;
    uint8_t registered_commands;
public:
    NetworkLayer() : registered_commands(0) {
        ACKSemaphore = xSemaphoreCreateBinary();
    }

    void registerLowerLayer(PHYInterface * phyInterface) {
        this->phyInterface = phyInterface;
    }

    void registerUpperLayer(FSXInterface * fsxInterface) {
        this->fsxInterface = fsxInterface;
    }

//    bool waitForACK(const uint32_t tickToWait) {
//        printf("ACK waiting\n");
//        bool res = xSemaphoreTake(ACKSemaphore, tickToWait);
//        if( res ) {
//            xSemaphoreGive(ACKSemaphore);
//            printf("ACK got!\n");
//        }
//        return res;
//    }

    void sendWithACK(const PHYDataStruct & data) {
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
        phyData.append(descriptors->table[registered_commands].ack);

        registered_commands++;
        sendWithACK(phyData);
    }


    virtual void passUp(const PHYDataStruct & data) {
        NETDataStruct netData(data.data[0], data.data+1, data.len-1);
        printf("[network] received (id = %d): ", netData.command);
        print_byte_table(netData.data, netData.len);
        if(netData.command == ACK_ID ) {
            printf("got ACK packet!\n");
            xSemaphoreGive(ACKSemaphore);
        }
        if( this->descriptors->at(netData.command).ack ) {
            printf("sending ACK!\n");
            NETDataStruct val = makeACKPacket();
            this->passDown(val);
        }
        fsxInterface->passUp(netData);
    }
};


#endif //PROTOCOL_NETWORKLAYER_H
