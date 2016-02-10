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

        ValuedDataDescriptor desc(descriptor);
        memcpy(desc.value.bytes, data.data, data.len);

//        std::cout << "Got value: " << descriptor.get(value) << std::endl;
        // TODO: what next?
    }

    virtual void passDown(const ValuedDataDescriptor & value) {
        NETDataStruct data(value.descriptor.id);
        printf("encode : %d; len: %d\n", value.descriptor.encode(), value.descriptor.length());
        data.append(value.value.bytes, value.descriptor.length());
        netInterface->passDown(data);
    }

    virtual void passDownRegistration(const DataDescriptor & descriptor) {
        NETDataStruct data(REGISTRATION_ID);
        data.append(descriptor.fsxId);
        data.append(descriptor.encode());
        printf("encode : %d; len: %d\n", descriptor.encode(), descriptor.length());
        netInterface->passDownRegistration(data);
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
