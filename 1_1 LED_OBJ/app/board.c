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
}
