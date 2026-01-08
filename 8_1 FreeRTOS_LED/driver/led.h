#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

// 描述LED灯的结构体/类
typedef struct LED_DESC_T{
	GPIO_TypeDef* led_port;	// LED对应的GPIO
	uint32_t led_pin;		// LED对应的GPIO的端口号
	BitAction led_onbit;	// 让LED亮的电平
	BitAction led_offbit;	// 让LED灭的电平
}led_desc_t;

// LED类的构造函数：略
// LED类的析构函数：略

// 对外暴露的方法接口（操作LED灯对象）
void initLed(led_desc_t* led);
void setLed(led_desc_t* led, uint8_t on_off);
void openLed(led_desc_t* led);
void closeLed(led_desc_t* led);

#endif
