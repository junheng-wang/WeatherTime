#include <stdio.h>

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "usart1.h"


// 定义一个函数指针变量
static console_received_func_t received_func;

/**
 *@brief USART1外设初始化
 *@note1 USART1_TX - PA9, USART1_RX - PA10
 */
void initUsart1(void)
{
	// 1. 开启时钟（board.c文件）
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// 2. 设置GPIO的复用功能
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	// 3. 配置GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// 4. 配置USART1
	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);

	// 5. 使能USART1
	USART_Cmd(USART1, ENABLE);
}

/**
 *@brief 通过USART1发送数据到电脑
 *@param str 要发送的字符串
 */
void sendDataByUsart1(const char* str)
{
	while(str && (*str != '\0'))
	{
		USART_SendData(USART1, *str);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);   //等待发送完成
		/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
		
		str++;
	}
}

/**
 *@brief 通过USART1从电脑接收一个字节的数据
 *@return 接收的一个字节数据
 */
uint8_t receiveDataByUsart1(void)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	return USART_ReceiveData(USART1);
}

int fputc(int ch, FILE *f)
{
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}

/**
 *@brief 注册USART1的接收回调函数
 */
void registerReceiveFunForUsart1(console_received_func_t func)
{
    received_func = func;
}
/**
 *@brief USART1的中断函数
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        if (received_func != NULL)
        {
            uint8_t data = USART_ReceiveData(USART1);
            received_func(data);
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
