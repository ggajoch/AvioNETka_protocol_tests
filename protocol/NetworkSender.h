//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_NETWORKSENDER_H
#define PROTOCOL_NETWORKSENDER_H

#include "DataStructs.h"
#include "PhysicalLayer.h"
#include "++FreeRTOS.h"

using namespace FreeRTOS;

class NetworkSender {
    PHYInterface * phy;
    Mutex mux;
public:
    NetworkSender(PHYInterface * phy_) {
        phy = phy_;
        mux.give();
    }
    void sendData(NetworkDataStruct & data) {
        mux.take();
        printf("[net]: sending data (id = %d): ", data.id);
        print_byte_table(data.data, data.len);
        PHYDataStruct phyData;
        phyData.append(data.id);
        phyData.append(data.data, data.len);
        phy->sendPacket(phyData);
        mux.give();
    }
};


#endif //PROTOCOL_NETWORKSENDER_H
