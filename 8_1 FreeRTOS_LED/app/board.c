#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "board.h"


// 实例化三个led对象
led_desc_t led1 = {GPIOE, GPIO_Pin_9, Bit_RESET, Bit_SET};
led_desc_t led2 = {GPIOB, GPIO_Pin_0, Bit_RESET, Bit_SET};
led_desc_t led3 = {GPIOB, GPIO_Pin_1, Bit_RESET, Bit_SET};

/**
 *@brief 初始化所有的用到的外设的时钟
 */
void initPeripheralRccClock(void)
{
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

}

/**
 *@brief 初始化所有用到的NVIC配置
 */
void initPeripheralNvic(void)
{
	// 设置中断优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 *@brief 板级底层初始化
 *@note1 这里用于所有外设时钟RCC和NVIC的初始化了
 */
void initBoardOnLowLevel(void)
{
	initPeripheralRccClock();
	initPeripheralNvic();
}

/**
 *@brief 初始化系统，包括外部模块、各种初始显示
 */
void initExternalDevices(void)
{
	// 2. 初始化USART1，用于日志调试printf
	initUsart1();
	
	printf("[SYS] Build Date: %s %s\n", __DATE__, __TIME__);
}
