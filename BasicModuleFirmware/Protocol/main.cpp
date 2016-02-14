#include <iostream>

#include <protocol/DataStructs.h>
#include <protocol/useful.h>
#include <protocol/NetworkLayer.h>
#include <protocol/TCPMock.h>


#include "++FreeRTOS.h"

using namespace std;
using namespace FreeRTOS;


#include "protocol/StackInterfaces.h"



NetworkLayer * net;


void StackPoll(void *p) {
    while(net == 0) {}
    while(1) {
        net->testConnection();
        vTaskDelay(100000);
    }
}
//class FSXLayer : public FSXInterface {
//    NETInterface * netInterface;
//public:
//    void registerLowerLayer(NETInterface * netInterface) {
//        this->netInterface = netInterface;
//    }
//    virtual void passUp(const NETDataStruct & data) {
//        printf("[FSX] Received: cmd = %d\n", data.command);
//        print_byte_table(data.data, data.len);
//    }
//    void mock(uint8_t command, uint32_t val) {
//        printf("[FSX] Sending: %d -> %d\n", command, val);
//        NETDataStruct netVal(command);
//        netVal.append(val);
//        this->netInterface->passDown(netVal);
//    }
//};

void ret(float x) {
    printf("Got value %f\n",x);
}

void starter(void * p) {
    TypedDataDescriptor<float> fsx(1, true,  ret);
    DataDescriptor * tab[] = {&fsx};
    NetworkLayer network(&phy, tab, 1);
    net = &network;

    while( net->stackState() != STACK_OK ) {
        vTaskDelay(100);
    }
//    net->sendSubscriptions();
//    fsx.send(1.0);

    vTaskDelay(10000);

    vTaskEndScheduler();
    while(1) {
        context::delay(portMAX_DELAY);
    }
}


class PHYSTDIO : public PHYInterface {

public:
    virtual StackError passDown(const PHYDataStruct &data) {
        printf("[PHY] sending data: ");
        print_byte_table(data.data, data.len);
        return STACK_OK;
    }
};


int main() {
    initSocket();

    Task::create(StackPoll, "poll", 1000, 2);
    Task::create(starter, "st", 1000, 2);
    TCP_SETUP();
//
    control::startScheduler();

    return 0;
}
