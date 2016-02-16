#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "utils/FIFO.h"
#include "uart.h"

/////////////////////////////////////////////////////////////////////////////////
#define USARTx USART3
#define ENABLE_CLOCKS() RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); \
                        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE)
#define GPIO_PORT (GPIOB)
#define GPIO_PINS (GPIO_Pin_10 | GPIO_Pin_11)
#define IRQ_CHANNEL USART3_IRQn
#define IRQ_HANDLER USART3_IRQHandler

#define CLASS_NAME USART3_Dev_t
#define DEV_NAME USART3_Dev

class CLASS_NAME {
public:
	FIFO_t<char, 50> Tx_FIFO;

	volatile uint8_t rx_overrun = 0;
	volatile uint8_t uart_transmit_restart = 1;

	void init(uint32_t baudrate) {
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStruct;
		NVIC_InitTypeDef NVIC_InitStructure;

		ENABLE_CLOCKS()
		;

		/* Configure USART pins:  Rx and Tx ----------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIO_PORT, &GPIO_InitStructure);

		USART_InitStruct.USART_BaudRate = baudrate;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
		USART_InitStruct.USART_StopBits = USART_StopBits_1;
		USART_InitStruct.USART_Parity = USART_Parity_No;
		USART_InitStruct.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(USARTx, &USART_InitStruct);

		NVIC_InitStructure.NVIC_IRQChannel = IRQ_CHANNEL;// we want to configure the USART1 interrupts
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // this sets the priority group of the USART1 interrupts
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	// the USART1 interrupts are globally enabled
		NVIC_Init(&NVIC_InitStructure);

		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt
		USART_ITConfig(USARTx, USART_IT_TXE, DISABLE); // enable the USART1 transmit empty interrupt
		uart_transmit_restart = 1;

		USART_Cmd(USARTx, ENABLE);
	}
//#undef putc
	void put_char(char data) {
		Tx_FIFO.push(data);
		if (uart_transmit_restart) {
			uart_transmit_restart = 0;
			USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
		}
	}
	void puts(const char *string) {
		while (*string) {
			this->put_char(*string);
			string++;
		}
	}
	void send(uint8_t * tab, uint8_t len) {
		for(uint8_t i = 0; i < len; ++i) {
			this->put_char(tab[i]);
		}
	}
};
static CLASS_NAME DEV_NAME;

#include "++FreeRTOS.h"
#include "../protocol/DataStructs.h"
static volatile char recvBuf[100], cmdBuf[100];
static volatile uint8_t buf_iterator = 0, cmd_iterator = 0;
FreeRTOS::Queue<PHYDataStruct> UART_Data_Rx_queue(100);


void send_to_queue(char now) {
	recvBuf[buf_iterator++] = now;

	if (now == 0xFF) {
		int i = 0;
		cmd_iterator = 0;
		buf_iterator = 0;

		while(recvBuf[i] != 0xFF) {
			uint8_t now = (recvBuf[i] << 4) | recvBuf[i+1];
			cmdBuf[cmd_iterator++] = now;
			i += 2;
		}
		PHYDataStruct data;
		memcpy((void *)data.data, (void *)cmdBuf+2, cmd_iterator-2);
		data.len = cmd_iterator-2;
		UART_Data_Rx_queue.sendToBack(data);
	}
}
extern "C" {
void USART3_IRQHandler(void) {
	if (USART_GetITStatus(USARTx, USART_IT_RXNE)) {
		//receive interrupt
		char now = USART_ReceiveData(USARTx);
		send_to_queue(now);
		USART_ClearITPendingBit(USARTx, USART_IT_RXNE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_TXE)) {
		//transmit interrupt
		if ( DEV_NAME.Tx_FIFO.size() > 0) {
			USART_SendData(USARTx, DEV_NAME.Tx_FIFO.pop());
		} else {
			DEV_NAME.uart_transmit_restart = 1;
			USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
		}
		USART_ClearITPendingBit(USARTx, USART_IT_TXE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_CTS)) {
		USART_ClearITPendingBit(USARTx, USART_IT_CTS);
	}
	if (USART_GetITStatus(USARTx, USART_IT_LBD)) {
		USART_ClearITPendingBit(USARTx, USART_IT_LBD);
	}
	if (USART_GetITStatus(USARTx, USART_IT_TC)) {
		USART_ClearITPendingBit(USARTx, USART_IT_TC);
	}
	if (USART_GetITStatus(USARTx, USART_IT_IDLE)) {
		USART_ClearITPendingBit(USARTx, USART_IT_IDLE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_ORE)) {
		USART_ClearITPendingBit(USARTx, USART_IT_ORE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_NE)) {
		USART_ClearITPendingBit(USARTx, USART_IT_NE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_FE)) {
		USART_ClearITPendingBit(USARTx, USART_IT_FE);
	}
	if (USART_GetITStatus(USARTx, USART_IT_PE)) {
		USART_ClearITPendingBit(USARTx, USART_IT_PE);
	}
}
}

extern "C" {
int __io_putchar(int ch) {
DEV_NAME.put_char((char) ch);
}
}

#endif//__UART_H
