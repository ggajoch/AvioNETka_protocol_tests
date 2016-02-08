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
public:
    NetworkLayer() {
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

    void sendData(const NETDataStruct & data) {
        PHYDataStruct phyData;
        phyData.append(data.command);
        phyData.append(data.data, data.len);
        phyInterface->passDown(phyData);
    }

    virtual void passDown(const NETDataStruct & data) {
        printf("[network] sending (id = %d): ", data.command);
        print_byte_table(data.data, data.len);

        if( this->descriptors->at(data.command).ack ) {
            for(uint8_t i = 0; i < 3; ++i) {
                printf("transmitting and waiting for ack\n");
                xSemaphoreTake(ACKSemaphore, 0);

                sendData(data);
                bool res = xSemaphoreTake(ACKSemaphore, 1000);
                res &= 0x1;
                if( res ) {
                    printf("ACK got!\n");
                    break;
                } else {
                    printf("PACKET LOSS!!!!\n");
                }
            }
        } else {
            sendData(data);
        }
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
            vTaskDelay(2000);
            NETDataStruct val = makeACKPacket();
            this->passDown(val);
        }
        fsxInterface->passUp(netData);
    }
};


#endif //PROTOCOL_NETWORKLAYER_H
