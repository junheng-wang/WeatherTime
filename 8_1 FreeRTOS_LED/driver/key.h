#ifndef __KEY_H__
#define __KEY_H__


#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"


typedef void (*key_func_t)(void);

typedef struct KET_T{
	GPIO_TypeDef* port;
	uint16_t pin;
	uint8_t exti_port_src;
	uint8_t exti_pin_src;
	uint32_t exti_line;
	uint8_t irqn;
	key_func_t func;
}key_t;


void initKey(key_t* key);
uint8_t readKey(key_t* key);
void key_press_callback_register(key_t* key, key_func_t func);

#endif
