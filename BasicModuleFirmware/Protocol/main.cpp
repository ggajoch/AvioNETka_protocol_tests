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
        vTaskDelay(2000);
    }
}

void ret(float x) {
    printf("Got value %f\n",x);
}

void starter(void * p) {
    TypedDataDescriptor<float> fsx(1, false,  ret);
    DataDescriptor * tab[] = {&fsx};
    NetworkLayer network(&phy, tab, 1);
    net = &network;

    printf("waiting for subscription packet\n");
    net->subscribeSemaphore.take(portMAX_DELAY);
    printf("sending subscriptions\n");
    net->subscribe();

    while (net->stackState() != STACK_OK) {
        vTaskDelay(100);
    }
    while(1) {
//        for(float i = 0; i < 100; i += 1) {
//            fsx.send(i);
//            vTaskDelay(10);
//        }
        vTaskDelay(10000);
    }

    vTaskEndScheduler();
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
