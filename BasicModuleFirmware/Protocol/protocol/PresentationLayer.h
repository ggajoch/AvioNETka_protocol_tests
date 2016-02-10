//
// Created by gregg on 2/10/2016.
//

#ifndef PROTOCOL_PRESENTATIONLAYER_H
#define PROTOCOL_PRESENTATIONLAYER_H

#include <iostream>
#include "DataStructs.h"
#include "StackInterfaces.h"
#include "ProtocolPackets.h"

class PresentationLayer : public PresentationInterface {
    const DataDescriptorsTable * descriptors;
    NETInterface * netInterface;
    PresentationInterface * presentationInterface;
public:
    virtual void passUp(const NETDataStruct & data) {
        const DataDescriptor & descriptor = this->descriptors->at(data.command);
        dataTypeUnion value;
        memcpy(value.bytes, data.data, data.len);

//        std::cout << "Got value: " << descriptor.get(value) << std::endl;
        // TODO: what next?
    }

    template<typename T>
    void passDown(const TypedDataDescriptor<T> & descriptor, const dataTypeUnion & value) {
        NETDataStruct data(descriptor.id);
        descriptor.pack(value);
        data.append(value.bytes, descriptor.length);
    }

    template<typename T>
    void passDownRegistration(const TypedDataDescriptor<T> & descriptor) {
        NETDataStruct data(REGISTRATION_ID);
        data.append(descriptor.fsxId);
        data.append(TypeEncoding<T>::get());
        netInterface->passDown(data);
    }

    void registerLowerLayer(NETInterface * netInterface) {
        this->netInterface = netInterface;
    }

    void registerUpperLayer(PresentationInterface * fsxInterface) {
//        this->fsxInterface = fsxInterface;
    }

    virtual void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
        this->netInterface->registerDataDescriptors(descriptors);
    }
};

#endif //PROTOCOL_PRESENTATIONLAYER_H
