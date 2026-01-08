#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "usart1.h"

uint8_t rx_data;

int main(void)
{
	initUsart1();
	uint32_t j = 1;
	while(1)
	{
		for(uint32_t i=0; i<2000000; i++);
		printf("Hello Num: %d\r\n", j);
		j = j>200 ? 0 : j+1;
		
		rx_data = receiveDataByUsart1();
		
		USART_SendData(USART1, rx_data);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);   //等待发送完成
		printf("\r\n");
	}
}

/**
 *@brief 重写fputc函数，用于printf发送USART1的数据
 */
int fputc(int ch, FILE* stream)
{
	(void)stream;
	USART_SendData(USART1, (uint16_t)ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);   //等待发送完成
	
	return ch;
}
