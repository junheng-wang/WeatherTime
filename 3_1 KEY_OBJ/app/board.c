#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "board.h"


// 实例化LED对象
led_desc_t led1 = {GPIOE, GPIO_Pin_9, Bit_RESET, Bit_SET};
led_desc_t led2 = {GPIOB, GPIO_Pin_1, Bit_RESET, Bit_SET};
led_desc_t led3 = {GPIOB, GPIO_Pin_0, Bit_RESET, Bit_SET};

// 实例化KEY对象
key_t key1 = {GPIOA, GPIO_Pin_0, EXTI_PortSourceGPIOA, EXTI_PinSource0, EXTI_Line0, EXTI0_IRQn, NULL};
key_t key2 = {GPIOC, GPIO_Pin_4, EXTI_PortSourceGPIOC, EXTI_PinSource4, EXTI_Line4, EXTI4_IRQn, NULL};
key_t key3 = {GPIOC, GPIO_Pin_5, EXTI_PortSourceGPIOC, EXTI_PinSource5, EXTI_Line5, EXTI9_5_IRQn, NULL};


/**
 *@brief 片上外设时钟初始化
 */
void initBoardLowLevel(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
}

/**
 *@brief key1按键对应的中断函数
 */
void EXTI0_IRQHandler(void)
{
    if (key1.func != NULL)
        key1.func();
	
    EXTI_ClearITPendingBit(EXTI_Line0);
}

/**
 *@brief key2按键对应的中断函数
 */
void EXTI4_IRQHandler(void)
{
    if (key2.func != NULL)
        key2.func();
	
    EXTI_ClearITPendingBit(EXTI_Line4);
}

/**
 *@brief key3按键对应的中断函数
 */
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5))
	{
		if (key3.func != NULL)
			key3.func();
		
		 EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

