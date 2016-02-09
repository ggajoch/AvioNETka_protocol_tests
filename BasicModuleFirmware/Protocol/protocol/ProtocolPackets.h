//
// Created by rexina on 04.12.2015.
//

#ifndef PROTOCOL_PROTOCOLPACKETS_H
#define PROTOCOL_PROTOCOLPACKETS_H

const int ACK_ID = 250;

static NETDataStruct makeACKPacket() {
    return NETDataStruct(ACK_ID, nullptr, 0);
}

const int SUBSCRIPTION_ID = 251;

static NETDataStruct makeSubscriptionPacket(DataDescriptor *descriptor) {
    NETDataStruct packet(SUBSCRIPTION_ID);
    packet.append(descriptor->id);
    packet.append(descriptor->fsxId);
    packet.append(descriptor->ack);
    packet.append(descriptor->rxEnabled);
    return packet;
}


#endif //PROTOCOL_PROTOCOLPACKETS_H
