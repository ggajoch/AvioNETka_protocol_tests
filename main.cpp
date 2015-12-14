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

FloatDataDescriptor data3({2, false});


xQueueHandle RxQueue, TxQueue;
class FreeRTOSMock : public PHYInterface {
    void sendPacket(PHYDataStruct & data) {
        printf("[RTOS] data receiving from module: ");
        print_byte_table(data.data, data.len);

        xQueueSend(TxQueue, &data, portMAX_DELAY);
    }
public:
    void mockData(PHYDataStruct * data) {
        printf("[RTOS] sending data to module: ");
        print_byte_table(data->data, data->len);
        net->dataReceived(*data);
    }
};


ApplicationLayer * app;
FreeRTOSMock PHYFree;


void TaskMockPC(void * p) {
    while(1) {
        PHYDataStruct data;
        if (xQueueReceive(TxQueue, &data, portMAX_DELAY)) {
            //data received from module
            printf("[MOCK]: data received!: ");
            print_byte_table(data.data, data.len);
            if (app->ackRequired(data.data[0])) {
                //send ACK
                printf("[MOCK] Sending ACK\n");
                //vTaskDelay(100);
                printf("[MOCK] Wait end\n");
                NetworkDataStruct ack = makeACKPacket();
                PHYDataStruct ph;
                ph.append(ack.id);
                ph.append(ack.data, ack.len);
                PHYFree.mockData(&ph);
            }
        }
        //vTaskDelay(1);
    }
}

void starter(void * p) {
    DataDescriptor * descriptors[] = {&data, &data2, &data3};
    ApplicationLayer appX(&PHYFree, descriptors, 3);
    app = &appX;

    printf("Hello!!!\n");
    PHYDataStruct data;
    data.data[0] = 252;
    data.len = 1;
    PHYFree.mockData(&data);

    xSemaphoreHandle x = xSemaphoreCreateBinary();
    xSemaphoreTake(x, 0);
    while(1) {
        xSemaphoreTake(x, portMAX_DELAY);
    }
}

void test(void * p) {
    portENTER_CRITICAL();
    xSemaphoreTake(app->link_established, portMAX_DELAY);
    xSemaphoreGive(app->link_established);
    portEXIT_CRITICAL();

    app->sendData(data, false);

    app->sendData(data, true);

    app->sendData(data2, 1.5f);
    while(1) {
        app->sendData(data2, 1.5f);
        printf("---------------------------\n\n");
        vTaskDelay(2000);
    }
    vTaskDelay(portMAX_DELAY);
}



int main() {
    RxQueue = xQueueCreate(100, sizeof(DataDescriptor));
    TxQueue = xQueueCreate(100, sizeof(DataDescriptor));

    xTaskCreate(TaskMockPC, "Rx", 1000, NULL, 3, NULL);
    xTaskCreate(starter, "st", 1000, NULL, 4, NULL);
    xTaskCreate(test, "ts", 1000, NULL, 1, NULL);

    vTaskStartScheduler();


    return 0;
}
