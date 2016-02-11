#ifndef PROTOCOL_STACKINTERFACES_H
#define PROTOCOL_STACKINTERFACES_H

#include "DataStructs.h"
class PHYInterface;
class NETInterface;
class PresentationInterface;
class ApplicationLayerInterface;

class PHYInterface {
protected:
    const DataDescriptorsTable * descriptors;
    const NETInterface * netInterface;
public:
    virtual void passDown(const PHYDataStruct & data) = 0;
    void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
    }

    void registerUpperLayer(NETInterface * netInterface) {
        this->netInterface = netInterface;
    }
};

class NETInterface {
protected:
    const DataDescriptorsTable * descriptors;
    PHYInterface * phyInterface;
    PresentationInterface * fsxInterface;
public:
    virtual void passUp(const PHYDataStruct & data) const = 0;
    virtual void passDown(const NETDataStruct & data) const = 0;
    virtual void passDownRegistration(const NETDataStruct & data) = 0;

    void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
        this->phyInterface->registerDataDescriptors(descriptors);
    }

    void registerLowerLayer(PHYInterface * phyInterface) {
        this->phyInterface = phyInterface;
    }

    void registerUpperLayer(PresentationInterface * fsxInterface) {
        this->fsxInterface = fsxInterface;
    }
};

class PresentationInterface {
protected:
    const DataDescriptorsTable * descriptors;
    NETInterface * netInterface;
    ApplicationLayerInterface * applicationLayerInterface;
public:
    virtual void passUp(const NETDataStruct & data) const = 0;
    virtual void passDown(const ValuedDataDescriptor & value) const = 0;
    virtual void passDownRegistration(const DataDescriptor & descriptor) = 0;

    void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
        this->netInterface->registerDataDescriptors(descriptors);
    }

    void registerLowerLayer(NETInterface * netInterface) {
        this->netInterface = netInterface;
    }

    void registerUpperLayer(ApplicationLayerInterface * applicationLayerInterface) {
        this->applicationLayerInterface = applicationLayerInterface;
    }
};

class ApplicationLayerInterface : public ApplicationDataInterface{
protected:
    const DataDescriptorsTable * descriptors;
    PresentationInterface * presentationInterface;
public:
    virtual void passUp(const ValuedDataDescriptor & data) const = 0;
    virtual void passDown(const ValuedDataDescriptor & value) const = 0;

    void registerDataDescriptors(const DataDescriptorsTable * const descriptors) {
        this->descriptors = descriptors;
        this->presentationInterface->registerDataDescriptors(descriptors);
    }

    void registerLowerLayer(PresentationInterface * presentationInterface) {
        this->presentationInterface = presentationInterface;
    }
};

#endif //PROTOCOL_STACKINTERFACES_H
