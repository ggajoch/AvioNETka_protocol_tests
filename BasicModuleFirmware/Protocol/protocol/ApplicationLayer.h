//
// Created by rexina on 03.12.2015.
//

#ifndef PROTOCOL_APPLICATIONLAYER_H
#define PROTOCOL_APPLICATIONLAYER_H


#include "StackInterfaces.h"

class ApplicationLayer : public ApplicationLayerInterface {
    const DataDescriptorsTable * descriptors;
    PresentationInterface * presentationInterface;
public:

    virtual void passUp(const ValuedDataDescriptor & data) {
        data.descriptor.callback(data.value);
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
        }
    }

    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
        this->presentationInterface->registerDataDescriptors(descriptors);
    }

    void registerLowerLayer(PresentationInterface * presentationInterface) {
        this->presentationInterface = presentationInterface;
    }
};


#endif //PROTOCOL_APPLICATIONLAYER_H
