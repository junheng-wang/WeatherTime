#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "led.h"
/**
 *@brief 初始化LED灯
 *@param led 指向描述led灯的结构体
 */
void initLed(led_desc_t* led)
{
	// 1. 开启LED对应的时钟：转到board_lowlevel_init()
	// 2. 初始化对应的GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = led->led_pin;	
	GPIO_Init(led->led_port, &GPIO_InitStruct);
}

/**
 *@brief 设置led灯的亮灭
 *@param led 指向描述led灯的结构体
 *@param on_off led灯的亮灭的标志
 */
void setLed(led_desc_t* led, uint8_t on_off)
{
	GPIO_WriteBit(led->led_port, led->led_pin, on_off ? led->led_onbit : led->led_offbit);
}

/**
 *@brief 设置led灯亮
 *@param led 指向描述led灯的结构体
 */
void openLed(led_desc_t* led)
{
	GPIO_WriteBit(led->led_port, led->led_pin, led->led_onbit);
}

/**
 *@brief 设置led灯灭
 *@param led 指向描述led灯的结构体
 */
void closeLed(led_desc_t* led)
{
	GPIO_WriteBit(led->led_port, led->led_pin, led->led_offbit);
}
