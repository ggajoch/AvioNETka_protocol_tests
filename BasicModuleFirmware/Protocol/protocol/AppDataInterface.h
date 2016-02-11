//
// Created by gregg on 2/11/2016.
//

#ifndef PROTOCOL_APPDATAINTERFACE_H
#define PROTOCOL_APPDATAINTERFACE_H


//#include "DataStructs.h"

class ValuedDataDescriptor;

class ApplicationDataInterface {
public:
    virtual void passDown(const ValuedDataDescriptor & value) const = 0;
};

#endif //PROTOCOL_APPDATAINTERFACE_H
