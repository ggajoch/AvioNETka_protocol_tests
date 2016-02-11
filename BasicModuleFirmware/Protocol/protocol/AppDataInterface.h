//
// Created by gregg on 2/11/2016.
//

#ifndef PROTOCOL_APPDATAINTERFACE_H
#define PROTOCOL_APPDATAINTERFACE_H


//#include "DataStructs.h"

class DataDescriptor;
union dataTypeUnion;

class ApplicationDataInterface {
public:
    virtual void send(const DataDescriptor & descriptor, dataTypeUnion value) = 0;
};

#endif //PROTOCOL_APPDATAINTERFACE_H
