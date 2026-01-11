#include "delay_tim6.h"
#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"


tim6_periodic_callback_t tim6_periodic_callback;
static volatile uint64_t tim6_count;


/**
 *@brief 初始化TIM6基本定时器
 *@note1 因为systick被FreeRTOS占用了，所以借用TIM6实现一个基本定时
 */
void initTim6(void)
{
	// 1. 开启TIM6的时钟(TIM6的时钟是82MHz)
	
	// 2. 获取时钟频率
	RCC_ClocksTypeDef RCC_ClocksStruct;
	RCC_GetClocksFreq(&RCC_ClocksStruct);
	uint32_t apb1_tim_freq_mhz = RCC_ClocksStruct.PCLK1_Frequency / 1000 / 1000 * 2;	// 乘以2是因为定时器的时钟是外设时钟的两倍
	
	// 3. 配置TIM6的工作模式
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 999;						// 计数到999后，自动回到0重新计数(在1MHz频率下计数1000个相当于1毫秒)
	TIM_TimeBaseInitStruct.TIM_Prescaler = apb1_tim_freq_mhz - 1;	// 预分频数，经过预分频后，TIM6的时钟变为1MHz吗？
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;			// 基础定时器不需要此参数
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
	
	// 4. 使能TIM6的中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	
	// 5. 使能TIM6
	TIM_Cmd(TIM6, ENABLE);

}

/**
 *@brief 获取当前TIM6进中断跳动的次数
 *@note1 注意，根据配置，1ms进一次中断
 *@return 变量tim6_count的值
 */
uint64_t getCurrentTim6Count(void)
{
	uint64_t last_tim6_count = tim6_count;
	// 因为tick_count每隔1ms才能在中断中更新一次，所以这里需要进行补偿
	uint64_t temp_count = tim6_count + TIM_GetCounter(TIM6);
	// 原子操作：防止计算到一半的时候进中断，导致这里没有更新
	while(last_tim6_count != tim6_count)
	{
		last_tim6_count = tim6_count;
		temp_count = tim6_count + TIM_GetCounter(TIM6);
	}
	
	return temp_count;
}

/**
 *@brief 获取经过的us数
 *@return us数
 */
uint64_t getPassedTimeUseUsUnit(void)
{
	return getCurrentTim6Count();
}

/**
 *@brief 获取经过的ms数
 *@return ms数
 */
uint64_t getPassedTimeUseMsUnit(void)
{
	return getCurrentTim6Count() / 1000;
}

/**
 *@brief 阻塞式微秒延时函数
 *@param us 延时的微秒数
 */
void delayUsUseTim6(uint32_t us)
{
	uint64_t temp_count = getCurrentTim6Count();
	while((getCurrentTim6Count() - temp_count) < (uint64_t)us); 
}

/**
 *@brief 阻塞式毫秒延时函数
 *@param ms 延时的毫秒数
 */
void delayMsUseTim6(uint32_t ms)
{
	uint64_t temp_count = getCurrentTim6Count();
	while((getCurrentTim6Count() - temp_count) < ((uint64_t)ms * 1000)); 	
}

/** 
 *@brief 注册回调函数
 */
void registerTim6PeriodicCallback(tim6_periodic_callback_t callback)
{
    tim6_periodic_callback = callback;
}


/**
 *@brief TIM6的中断函数
 */
void TIM6_DAC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		// 每隔1us，数字加1000
		tim6_count += 1000;
		// 执行回调函数
		if(tim6_periodic_callback != NULL)
			tim6_periodic_callback();
	}
}
