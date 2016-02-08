//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_APPLICATIONLAYER_H
#define PROTOCOL_APPLICATIONLAYER_H

#include "NetworkLayer.h"
#include "PhysicalLayer.h"
#include "DataStructs.h"
#include "ProtocolPackets.h"

/*class ApplicationLayer : public ApplicationLayerInterface {
    NetworkLayer net;
    DataDescriptor **descriptors;
    uint8_t descriptors_length;

    void sendSubscriptions() {
        for (uint8_t i = 0; i < descriptors_length; ++i) {
            net.sendData(makeSubscriptionPacket(descriptors[i]));
        }
    }

public:
    xSemaphoreHandle link_established;


    ApplicationLayer(PHYLayer *phy, DataDescriptor **table, uint8_t len) :
            net(this, phy), descriptors(table), descriptors_length(len) {

        link_established = xSemaphoreCreateBinary();
        xSemaphoreTake(link_established, 0);

        for (uint8_t i = 0; i < len; ++i) {
            descriptors[i]->id = i;
            printf("assigning ID %d\n", i);
        }
    }

    template<typename T>
    void sendData(const T & data, typename T::type value) {
        dataTypeUnion packedData = data.pack(value);
        NETDataStruct netData(data.id, packedData.bytes, T::length);
        net.sendData(netData);
    }

    void dataReceived(NETDataStruct data) {
        if (data.command < descriptors_length) {
            dataTypeUnion value;
            memcpy(value.bytes, data.data, data.len);
            descriptors[data.command]->callback(value);
        } else {
            if(data.command == 252 ) { //[TODO]: remove magic number
                this->sendSubscriptions();
                xSemaphoreGive(link_established);
            }
        }
    }

    bool ackRequired(uint8_t id) {
        bool res = false;
        if (id < descriptors_length) {
            res = descriptors[id]->ack;
        } else {
            switch(id) {
                case 251:
                case 252:
                    res = true;
                    break;
                default:
                    res = false;
            }
        }
        printf("ACK id = %d? -> %d\n", id, res);
        return res;
    }
};*/


#endif //PROTOCOL_APPLICATIONLAYER_H
