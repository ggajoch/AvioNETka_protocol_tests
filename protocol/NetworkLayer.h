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
#include "NetworkSender.h"
#include "NetworkReceiver.h"
#include "ProtocolPackets.h"

class ApplicationLayerInterface {
public:
    virtual void dataReceived(NetworkDataStruct) = 0;
    virtual bool ackRequired(uint8_t id) = 0;
};

class NetworkLayer : public NetworkRcvInterface {
    ApplicationLayerInterface * app;
    PHYInterface * phy;
    NetworkSender sender;
    NetworkReceiver receiver;
    volatile bool ackgot = true;
    SemaphoreHandle_t ACKSemaphore;
    EventGroupHandle_t link_state;
public:
    NetworkLayer(ApplicationLayerInterface *app, PHYInterface *phy) :
            app(app), phy(phy), sender(phy), receiver(this) {
        phy->attachNetwork(&receiver);
        ACKSemaphore = xSemaphoreCreateBinary();
        link_state = xEventGroupCreate();
        xEventGroupClearBits(link_state, 0x1);
        xSemaphoreGive(ACKSemaphore);
        printf("created network node\n");
    }
    bool waitForACK(const uint32_t tickToWait) {
        printf("ACK waiting\n");
        bool res = xSemaphoreTake(ACKSemaphore, tickToWait);
        if( res ) {
            xSemaphoreGive(ACKSemaphore);
            printf("ACK got!\n");
        }
        return res;
    }
    void sendData(NetworkDataStruct data) {
        printf("[network] sending (id = %d): ", data.id);
        print_byte_table(data.data, data.len);

        if( app->ackRequired(data.id) ) {
            for(uint8_t i = 0; i < 3; ++i) {
                printf("transmitting and waiting for ack\n");
                //xSemaphoreTake(ACKSemaphore, 0);

                sender.sendData(data);
                //bool res = xSemaphoreTake(ACKSemaphore, 1000);
                bool res = xEventGroupWaitBits(link_state, 1, true, true, 0);
                xEventGroupClearBits(link_state, 0x1);
                res &= 0x1;
                if( res ) {
                    printf("ACK got!\n");
                    break;
                }
                /*if( waitForACK(1000) ) {
                    break;
                } else {
                    printf("PACKET LOSS!!!!\n");
                }*/
            }
        } else {
            sender.sendData(data);
        }
    }
    void dataReceived(NetworkDataStruct data) {
        printf("[network] received (id = %d): ", data.id);
        print_byte_table(data.data, data.len);
        if( data.id == ACK_ID ) {
            printf("got ACK packet!\n");
            //ackgot = true;
            //xSemaphoreGive(ACKSemaphore);
            xEventGroupSetBits(link_state, 1);
        }
        if( app->ackRequired(data.id) ) {
            printf("sending ACK!\n");
            NetworkDataStruct val = makeACKPacket();
            sender.sendData(val);
        }
        app->dataReceived(data);
    }
    static void linkManagementTask(void * ptr) {

    }
};


#endif //PROTOCOL_NETWORKLAYER_H
