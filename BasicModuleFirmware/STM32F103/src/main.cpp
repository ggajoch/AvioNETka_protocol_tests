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

extern "C" {
	#include "Delay/delay_ms.h"
	void SysTick_Handler() {
		DelayDecrement();
	}
}

GPIO_t LED(GPIOC, GPIO_Pin_12);

static char buf[100];
int main(void) {
	SysTick_Config(SystemCoreClock/10000);
	USART3_Dev.init(115200);
	float x = 0;
	for(;;) {
		_delay_ms(1000);
		LED.toogle();
		USART3_Dev.puts("hello!\r\n");
		sprintf(buf, "AAA %f\r\n", x);
		x += 0.5;
		USART3_Dev.puts(buf);
		printf("XXX %f\r\n", x);
	}
}
