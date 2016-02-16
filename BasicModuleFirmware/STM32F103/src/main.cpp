/**
  ******************************************************************************
 * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

//#define FLOATING_POINT
#include <stdio.h>

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "GPIO/GPIO.h"

#include "UART/uart.h"


#include "FreeRTOS/++FreeRTOS.h"
using namespace FreeRTOS;


#include "../protocol/DataStructs.h"
#include "../protocol/useful.h"
#include "../protocol/NetworkLayer.h"


#include "../protocol/StackInterfaces.h"

#include "UARTPHY.h"

NetworkLayer * net;

void StackPoll(void *p) {
    while(net == 0) {}
    while(1) {
        net->testConnection();
        vTaskDelay(10000);
    }
}

void ret(float x) {
//    printf("Got value %f\n",x);
}

void ret2(uint8_t x) {
//    printf("Got value %f\n",x);
}

TypedDataDescriptor<float> * throttle;

void starter(void * p) {

	TypedDataDescriptor<float> fsx(1, true,  ret);
	TypedDataDescriptor<uint8_t> fsx2(1, false,  ret2);
	throttle = &fsx;
	DataDescriptor * tab[] = {&fsx, &fsx2};
	NetworkLayer network(&phy, tab, 2);
	net = &network;
	while(1) {
		if( net->subscribeSemaphore.take(portMAX_DELAY) ) {
			net->subscribe();
		}
		while (net->stackState() != STACK_OK) {
			vTaskDelay(100);
		}
	}
}



GPIO_t LED(GPIOC, GPIO_Pin_12);

void blink(void * p) {
	/*GPIO_t y(GPIOB, GPIO_Pin_11);
	for(int i = 0; i < 100; ++i) {
		y.toogle();
		context::delay(10);

	}
	y.on();*/

	float x = 0;
	while(1) {
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET) {
			context::delay(1);
		}
		LED.toogle();
//		USART3_Dev.puts("hello!\r\n");

		throttle->send(x);
		x += 5.0;
		if( x > 100 ) {
			x = 0;
		}
//		context::delay(100);
	}
}

static char buf[100];
int main(void) {

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);


	USART3_Dev.init(115200);
	FreeRTOS::Task::create(blink, "blink", 200, 3);
	Task::create(StackPoll, "poll", 200, 2);
	Task::create(starter, "st", 500, 2);

	UART_SETUP();

	control::startScheduler();
	float x = 0;
	for(;;) {
//		_delay_ms(1000);

//		USART3_Dev.puts("hello!\r\n");
		sprintf(buf, "AAA %f\r\n", x);
		x += 0.5;
//		USART3_Dev.puts(buf);
//		printf("XXX %f\r\n", x);
	}
}

extern "C" {
void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    signed char *pcTaskName ) {
	while(1) {}
}
}
