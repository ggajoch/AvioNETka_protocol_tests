#ifndef PROTOCOL_STACK_H
#define PROTOCOL_STACK_H

#include "StackInterfaces.h"
#include "NetworkLayer.h"
#include "PresentationLayer.h"
#include "ApplicationLayer.h"



class Stack {
    ApplicationLayer app;
    PresentationLayer pres;
    NetworkLayer net;
    DataDescriptorsTable table;
public:

    Stack(PHYInterface & phy, DataDescriptor ** desc, const uint8_t len) {
        app.registerLowerLayer(&pres);
        pres.registerUpperLayer(&app);
        pres.registerLowerLayer(&net);
        net.registerUpperLayer(&pres);
        net.registerLowerLayer(&phy);


        table = DataDescriptorsTable(desc, 6);
        app.registerDataDescriptors(&table);
        app.sendSubscriptions();
    }
};


#endif //PROTOCOL_STACK_H
