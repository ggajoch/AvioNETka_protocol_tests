#ifndef PROTOCOL_STACKINTERFACES_H
#define PROTOCOL_STACKINTERFACES_H

#include "DataStructs.h"

class PHYInterface {
public:
    virtual void passDown(const PHYDataStruct & data) = 0;
    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {

    }
};

class NETInterface {
public:
    virtual void passUp(const PHYDataStruct & data) = 0;
    virtual void passDown(const NETDataStruct & data) = 0;
    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) = 0;
    virtual void passDownRegistration(const NETDataStruct & data) = 0;
};

class PresentationInterface {
public:
    virtual void passUp(const NETDataStruct & data) = 0;
    virtual void passDown(const ValuedDataDescriptor & value) = 0;
    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) = 0;
    virtual void passDownRegistration(const DataDescriptor & descriptor) = 0;
};

class ApplicationLayerInterface {
public:
    virtual void passUp(const ValuedDataDescriptor & data) = 0;
//    virtual void passDown(const ValuedDataDescriptor & value) = 0;
    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) = 0;
//    virtual void passDownRegistration(const DataDescriptor & descriptor) = 0;
};

#endif //PROTOCOL_STACKINTERFACES_H
