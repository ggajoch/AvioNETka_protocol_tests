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
public:
    NetworkLayer(ApplicationLayerInterface *app, PHYInterface *phy) :
            app(app), phy(phy), sender(phy), receiver(this) {
        phy->attachNetwork(&receiver);
        ACKSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(ACKSemaphore);
        printf("created network node\n");
    }
    void waitForACK() {
        printf("ACK waiting\n");
        xSemaphoreTake(ACKSemaphore, portMAX_DELAY);
        xSemaphoreGive(ACKSemaphore);
        printf("ACK got!\n");
    }
    void sendData(NetworkDataStruct data) {
        printf("[network] sending (id = %d): ", data.id);
        print_byte_table(data.data, data.len);

        if( app->ackRequired(data.id) ) {
            printf("transmitting and waiting for ack\n");
            xSemaphoreTake(ACKSemaphore, portMAX_DELAY);
            sender.sendData(data);
            waitForACK();
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
            xSemaphoreGive(ACKSemaphore);
        }
        if( app->ackRequired(data.id) ) {
            printf("sending ACK!\n");
            NetworkDataStruct val = makeACKPacket();
            sender.sendData(val);
        }
        app->dataReceived(data);
    }
};


#endif //PROTOCOL_NETWORKLAYER_H
