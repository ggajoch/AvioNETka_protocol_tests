//
// Created by rexina on 02.12.2015.
//

#ifndef PROTOCOL_NETWORKRECEIVER_H
#define PROTOCOL_NETWORKRECEIVER_H

#include "useful.h"
#include "DataStructs.h"
//#include "NetworkLayer.h"

class NetworkRcvInterface {
public:
    virtual void dataReceived(NetworkDataStruct data) = 0;
};

class NetworkReceiver {
    NetworkRcvInterface * net;
public:
    NetworkReceiver(NetworkRcvInterface * net_) {
        net = net_;
    }
    void dataReceived(PHYDataStruct& data) {
        printf("[net]: data received |%d|: ",data.len);
        print_byte_table(data.data, data.len);
        NetworkDataStruct pass(data.data[0], data.data+1, data.len-1);
        net->dataReceived(pass);
    }
};


#endif //PROTOCOL_NETWORKRECEIVER_H
