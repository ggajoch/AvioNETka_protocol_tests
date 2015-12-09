//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_APPLICATIONLAYER_H
#define PROTOCOL_APPLICATIONLAYER_H

#include "NetworkLayer.h"
#include "PhysicalLayer.h"
#include "DataStructs.h"
#include "ProtocolPackets.h"

class ApplicationLayer : public ApplicationLayerInterface {
    NetworkLayer net;
    DataDescriptor **descriptors;
    uint8_t descriptors_length;

    void sendSubscriptions() {
        for (uint8_t i = 0; i < descriptors_length; ++i) {
            net.sendData(makeSubscriptionPacket(descriptors[i]));
        }
    }

public:
    ApplicationLayer(PHYInterface *phy, DataDescriptor **table, uint8_t len) :
            net(this, phy), descriptors(table), descriptors_length(len) {
        for (uint8_t i = 0; i < len; ++i) {
            descriptors[i]->id = i;
            printf("assigning ID %d\n", i);
        }
        //sendSubscriptions();
    }
    void _sendSubscriptions() {
        this->sendSubscriptions();
    }

    template<typename T>
    void sendData(T data, typename T::type value) {
        dataTypeUnion packedData = data.pack(value);
        NetworkDataStruct netData(data.id, packedData.bytes, T::length);
        net.sendData(netData);
    }

    void dataReceived(NetworkDataStruct data) {
        if (data.id < descriptors_length) {
            dataTypeUnion value;
            memcpy(value.bytes, data.data, data.len);
            descriptors[data.id]->callback(value);
        }
    }

    bool ackRequired(uint8_t id) {
        bool res = false;
        if (id < descriptors_length) {
            res = descriptors[id]->ack;
        } else {
            switch(id) {
                case 251:
                    res = true;
                    break;
                default:
                    res = false;
            }
        }
        printf("ACK id = %d? -> %d\n", id, res);
        return res;
    }
};


#endif //PROTOCOL_APPLICATIONLAYER_H
