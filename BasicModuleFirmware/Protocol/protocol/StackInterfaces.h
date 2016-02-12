#ifndef PROTOCOL_STACKINTERFACES_H
#define PROTOCOL_STACKINTERFACES_H

#include "DataStructs.h"

class PHYInterface;
class NETInterface;

class PHYInterface {
protected:
    const NETInterface * netInterface;
public:
    virtual void passDown(const PHYDataStruct & data) = 0;

    void registerUpperLayer(NETInterface * netInterface) {
        this->netInterface = netInterface;
    }
};

class NETInterface {
protected:
    PHYInterface * phyInterface;
public:
    virtual void passUp(const PHYDataStruct & data) const = 0;
    virtual void passDown(const NETDataStruct & data) const = 0;
    virtual void passDownWithACK(const NETDataStruct & data) const = 0;

    void registerLowerLayer(PHYInterface * phyInterface) {
        this->phyInterface = phyInterface;
    }
};

#endif //PROTOCOL_STACKINTERFACES_H
