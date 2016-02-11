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
public:

    Stack(PHYInterface & phy, DataDescriptorsTable * desc) {
        app.registerLowerLayer(&pres);
        pres.registerUpperLayer(&app);
        pres.registerLowerLayer(&net);
        net.registerUpperLayer(&pres);
        net.registerLowerLayer(&phy);

        app.registerDataDescriptors(desc);

        app.sendSubscriptions();
    }
};


#endif //PROTOCOL_STACK_H
