#ifndef PROTOCOL_PHYSICALLAYER_H
#define PROTOCOL_PHYSICALLAYER_H

#include <stdint-gcc.h>
#include <stdio.h>
#include <string.h>

#include "DataStructs.h"
#include "useful.h"
#include "StackInterfaces.h"




class PHYLayer : public PHYInterface {
protected:
    NETInterface *  net = 0;
public:
    PHYLayer() {}
    void registerUpperLayer(NETInterface * net) {
        this->net = net;
    }
    virtual void passDown(const PHYDataStruct & data) {
        printf("[phy] sending packet: ");
        print_byte_table(data.data, data.len);
    }
};


#endif //PROTOCOL_PHYSICALLAYER_H
