#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "board.h"


// 实例化LED对象
led_desc_t led1 = {GPIOE, GPIO_Pin_9, Bit_RESET, Bit_SET};
led_desc_t led2 = {GPIOB, GPIO_Pin_1, Bit_RESET, Bit_SET};
led_desc_t led3 = {GPIOB, GPIO_Pin_0, Bit_RESET, Bit_SET};


/**
 *@brief 片上外设时钟初始化
 */
void initBoardLowLevel(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
}

/**
 *@brief 初始化所有用到的NVIC配置
 */
void initPeripheralNvic(void)
{
	// 设置中断优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	// 配置TIM6_DAC_IRQn
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
}
