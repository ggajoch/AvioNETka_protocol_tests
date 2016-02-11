//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_APPLICATIONLAYER_H
#define PROTOCOL_APPLICATIONLAYER_H


#include "StackInterfaces.h"
#include "AppDataInterface.h"

class ApplicationLayer : public ApplicationLayerInterface {
public:

    virtual void passUp(const ValuedDataDescriptor & data) const {
        data.descriptor.callback(data.value);
    }

    virtual void passDown(const ValuedDataDescriptor & value) const {
        this->presentationInterface->passDown(value);
    }

    template <typename T>
    void send(const TypedDataDescriptor<T> & descriptor, T value) {
        ValuedDataDescriptor valuedDataDescriptor(descriptor);
        valuedDataDescriptor.value = descriptor.pack(value);
        this->presentationInterface->passDown(valuedDataDescriptor);
    }


    void sendSubscriptions() {
        for(int i = 0; i < descriptors->len; ++i) {
            presentationInterface->passDownRegistration(descriptors->at(i));
            descriptors->at(i).app = this;
        }
    }
};


#endif //PROTOCOL_APPLICATIONLAYER_H
