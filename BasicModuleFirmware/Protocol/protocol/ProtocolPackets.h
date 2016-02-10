//
// Created by rexina on 04.12.2015.
//

#ifndef PROTOCOL_PROTOCOLPACKETS_H
#define PROTOCOL_PROTOCOLPACKETS_H

#include "StackInterfaces.h"

const int ACK_ID = 250;
const int REGISTRATION_ID = 255;

static NETDataStruct makeACKPacket() {
    return NETDataStruct(ACK_ID, nullptr, 0);
}


#endif //PROTOCOL_PROTOCOLPACKETS_H
