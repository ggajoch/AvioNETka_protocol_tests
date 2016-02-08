#ifndef PROTOCOL_STACKINTERFACES_H
#define PROTOCOL_STACKINTERFACES_H

#include <array>
#include "DataStructs.h"

class PHYInterface {
public:
    virtual void passDown(const PHYDataStruct & data) = 0;
};

class NETInterface {
protected:
    const DataDescriptorsTable * descriptors;
public:
    virtual void passUp(const PHYDataStruct & data) = 0;
    virtual void passDown(const NETDataStruct & data) = 0;
    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
    }
};

class FSXInterface {
public:
    virtual void passUp(const NETDataStruct & data) = 0;
//    virtual void passDown(const FSXDataStruct & data) = 0;
};

#endif //PROTOCOL_STACKINTERFACES_H
