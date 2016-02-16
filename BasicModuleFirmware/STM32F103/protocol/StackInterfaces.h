#ifndef PROTOCOL_STACKINTERFACES_H
#define PROTOCOL_STACKINTERFACES_H

#include "DataStructs.h"
#include "stackError.h"

class PHYInterface;
class NETInterface;

class PHYInterface {
protected:
    NETInterface * netInterface;
public:
    virtual StackError passDown(const PHYDataStruct & data) = 0;

    void registerUpperLayer(NETInterface * netInterface) {
        this->netInterface = netInterface;
    }
};

class NETInterface {
protected:
    PHYInterface * phyInterface;
public:
    virtual StackError stackState() = 0;
    virtual StackError passUp(const PHYDataStruct & data) = 0;
    virtual StackError passDown(const NETDataStruct & data) = 0;
    virtual StackError passDownWithACK(const NETDataStruct & data) = 0;

    virtual StackError markSubscriptions() = 0;
    virtual void receivedACK() = 0;

    void registerLowerLayer(PHYInterface * phyInterface) {
        this->phyInterface = phyInterface;
    }
};

#endif //PROTOCOL_STACKINTERFACES_H
