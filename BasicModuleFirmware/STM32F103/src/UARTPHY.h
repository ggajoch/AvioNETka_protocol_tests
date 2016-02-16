#ifndef UARTPHY_H_
#define UARTPHY_H_

#include "UART/uart.h"

#include "++FreeRTOS.h"
#include "../protocol/StackInterfaces.h"
using namespace FreeRTOS;

static Queue<PHYDataStruct> TxQueue(100);

class UART_PHYLayer : public PHYInterface {
public:
    void passUp(const PHYDataStruct & data) {
//        printf("[PHY] received data: ");
        print_byte_table(data.data, data.len);
        netInterface->passUp(data);
    }

    virtual StackError passDown(const PHYDataStruct & data) {
        TxQueue.sendToBack(data);
        return STACK_OK;
    }
};

UART_PHYLayer phy;


extern Queue<PHYDataStruct> UART_Data_Rx_queue;
void UART_Receiver_Queue(void *p) {
    while(1) {
        static PHYDataStruct data;
        UART_Data_Rx_queue.receive(&data, portMAX_DELAY);
        phy.passUp(data);
    }
}

void UART_Sender_Queue(void * p) {
    while(1) {
    	static PHYDataStruct data;
        TxQueue.receive(&data, portMAX_DELAY);
        static uint8_t buffer[100];
        static uint8_t bufferOut[100];
        uint16_t address = 0xAAAA;
        memcpy(buffer, &address, 2);
        memcpy(buffer + 2, data.data, data.len);

//        printf("[TCP] sending data: ");
//        print_byte_table(buffer, data.len + 2);

        int len = data.len+2;
        int iter = 0;
        for(uint8_t i = 0; i < len; ++i) {
            bufferOut[iter++] = (buffer[i] & 0xF0) >> 4;
            bufferOut[iter++] = buffer[i] & 0x0F;
        }
        bufferOut[iter++] = 0xFF;

        USART3_Dev.send(bufferOut, iter);
    }
}

void UART_SETUP() {
    Task::create(UART_Receiver_Queue, "rx", 300, 3);
    Task::create(UART_Sender_Queue, "tx", 300, 3);
}


#endif /* UARTPHY_H_ */
