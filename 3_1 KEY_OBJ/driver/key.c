#include "key.h"

/*
// KEY1: PA0
// KEY2: PC4
// KEY3: PC5

//#define KEY_DEFINE(n, PORT, PIN, IRQN) \
//static key_t key_func_t##n = \
//{ \
//	.port = GPIO##PORT, \
//	.pin = GPIO_Pin_##PIN, \
//	.exti_port_src = EXTI_PortSourceGPIO##PORT, \
//	.exti_pin_src = EXTI_PinSource##PIN, \
//	.exti_line = EXTI_Line##PIN, \
//	.irqn = IRQN, \
//	.func = NULL \
//};
// 根据宏定义实例化三个Key类对象
// KEY_DEFINE(1, A, 0, EXTI0_IRQn)
// KEY_DEFINE(2, C, 4, EXTI4_IRQn)
// KEY_DEFINE(3, C, 5, EXTI9_5_IRQn)

// 定义三个按键的回调函数
static key_func_t key1_func, key2_func, key3_func;
*/

/**
 *@brief 按键初始化
 */
void initKey(key_t* key)
{
	// 1. 开启对应的GPIO时钟(Key1-PA0, Key2-PC4, Key3-PC5)
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	// 2. 开启中断线连接
	SYSCFG_EXTILineConfig(key->exti_port_src, key->exti_pin_src);
	
	// 3. 配置GPIO的工作模式
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = key->pin;
	GPIO_Init(key->port, &GPIO_InitStruct);
	
	// 4. EXTI中断初始化
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_StructInit(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Line = key->exti_line;
    EXTI_Init(&EXTI_InitStructure);
	
	// 5. NVIC中断优先级配置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = key->irqn;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 *@brief 读取按键对应的GPIO口当前的电平
 *@param idx 按键对应的编号
 *@return 1-对应的按键被按下，0-按键没有被按下
 */
uint8_t readKey(key_t* key)
{
	return GPIO_ReadInputDataBit(key->port, key->pin);
}

/**
 *@brief 为指定的按键注册回调函数
 *@param key 指定按键
 *@param func 被注册的回调函数
 */
void key_press_callback_register(key_t* key, key_func_t func)
{
	key->func = func;
}
