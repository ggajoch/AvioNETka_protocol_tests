#include <iostream>
#include <protocol/NetworkSender.h>
#include <protocol/NetworkLayer.h>
#include <protocol/ApplicationLayer.h>


extern "C" {
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
}

using namespace std;


void firstCall(bool x) {
    printf("got value %d\n",x);
}
BoolDataDescriptor data({1, true}, firstCall);

void secondCall(float x) {
    printf("got value %f\n",x);
}
FloatDataDescriptor data2({2, true}, secondCall);

FloatDataDescriptor data3({2, true});


xQueueHandle RxQueue, TxQueue;
class FreeRTOSMock : public PHYInterface {
    void sendPacket(PHYDataStruct & data) {
        printf("[RTOS] sending packet: ");
        print_byte_table(data.data, data.len);

        xQueueSend(TxQueue, &data, portMAX_DELAY);
    }
public:
    void mockData(PHYDataStruct * data) {
        printf("MOCKING DATA\n");
        net->dataReceived(*data);
    }
};


ApplicationLayer * app;
FreeRTOSMock PHYFree;

void taskTx(void * p) {
    while(1) {
        PHYDataStruct data;
        xQueueReceive(TxQueue, &data, portMAX_DELAY);
        printf("[RTOS] checking packet: %d\n", app->ackRequired(data.data[0]));

        if( app->ackRequired(data.data[0]) ) {
            printf("[RTOS] sending ACK\n");
            //send ACK
            NetworkDataStruct ack = makeACKPacket();
            PHYDataStruct ph;
            ph.append(ack.id);
            ph.append(ack.data, ack.len);
            xQueueSend(RxQueue, &ph, portMAX_DELAY);
        }
        xQueueSend(RxQueue, &data, portMAX_DELAY);
    }
}

void taskRx(void * p) {
    int c = 5;
    while(1) {
        PHYDataStruct data;
        xQueueReceive(RxQueue, &data, portMAX_DELAY);
        printf("[RTOS] receiving packet!!!\n");
        vTaskDelay(1000);
        PHYFree.mockData(&data);
    }
}

void starter(void * p) {
    DataDescriptor * descriptors[] = {&data, &data2, &data3};
    ApplicationLayer appX(&PHYFree, descriptors, 3);
    app = &appX;
    app->_sendSubscriptions();
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}

void test(void * p) {
    app->sendData(data, false);

    app->sendData(data, true);

    app->sendData(data2, 1.5f);
    while(1) {
        printf("\n\n\n---------------------------------\n\n\n");
        app->sendData(data2, 1.5f);
        vTaskDelay(5000);
    }
    vTaskDelay(portMAX_DELAY);
}

int main() {
    RxQueue = xQueueCreate(100, sizeof(DataDescriptor));
    TxQueue = xQueueCreate(100, sizeof(DataDescriptor));

    xTaskCreate(taskRx, "Rx", 1000, NULL, 5, NULL);
    xTaskCreate(taskTx, "Rx", 1000, NULL, 2, NULL);
    xTaskCreate(starter, "st", 1000, NULL, 4, NULL);
    xTaskCreate(test, "ts", 1000, NULL, 1, NULL);

    vTaskStartScheduler();



    /*app.sendData(data, false);

    app.sendData(data, true);

    app.sendData(data2, 1.5f);*/


    return 0;
}
