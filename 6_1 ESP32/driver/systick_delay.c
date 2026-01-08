#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "systick_delay.h"


#define TICKS_PER_MS    (SystemCoreClock / 1000)
#define TICKS_PER_US    (SystemCoreClock / 1000000)


// 变量：用于累加systick跳动的次数，跳一次的时间是1/SystemCoreClock = 1/168000000（秒）
static volatile uint64_t tick_count;


/**
 *@brief 中断式(非阻塞)的1ms固定延时
 */
void delayOneMsUseInterruptModeInit(void)
{
	SysTick->LOAD = TICKS_PER_MS;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/**
 *@brief 获取当前systick跳动的次数
 *@return 变量tick_count的值
 */
uint64_t getCurrentTickCount(void)
{
	uint64_t last_tick_count = tick_count;
	// 因为tick_count每隔1ms才能在中断中更新一次，所以这里需要进行补偿
	uint64_t temp_count = tick_count + (SysTick->LOAD - SysTick->VAL);
	// 原子操作：防止计算到一半的时候进中断，导致这里没有更新
	while(last_tick_count != tick_count)
	{
		last_tick_count = tick_count;
		temp_count = tick_count + (SysTick->LOAD - SysTick->VAL);
	}
	
	return temp_count;
}

/**
 *@brief 获取经过的us数
 *@return us数
 */
uint64_t getPassedTimeUseUsUnit(void)
{
	return getCurrentTickCount() / TICKS_PER_US;
}

/**
 *@brief 获取经过的ms数
 *@return ms数
 */
uint64_t getPassedTimeUseMsUnit(void)
{
	return getCurrentTickCount() / TICKS_PER_MS;
}

/**
 *@brief 阻塞式微秒延时函数
 *@param us 延时的微秒数
 */
void delayUsUseSystick(uint32_t us)
{
	uint64_t temp_count = getCurrentTickCount();
	while((getCurrentTickCount() - temp_count) < ((uint64_t)us * TICKS_PER_US)); 
}

/**
 *@brief 阻塞式毫秒延时函数
 *@param ms 延时的毫秒数
 */
void delayMsUseSystick(uint32_t ms)
{
	uint64_t temp_count = getCurrentTickCount();
	while((getCurrentTickCount() - temp_count) < ((uint64_t)ms * TICKS_PER_MS)); 	
}

/**
 *@brief Systick的中断函数
 */
void SysTick_Handler(void)
{
	// 进来一次中断，说明计数值跳动了168000次（即1ms），所以将变量增加168000
	tick_count = tick_count + TICKS_PER_MS;
}

// =====================================================================================================================
// =====================================================================================================================


/**
 *@brief 阻塞式微秒延时函数
 *@param us 延时的微秒数
 */
void delay_us(uint32_t us)
{
	while(us > 1000)
	{
		// SystemCoreClock是
		SysTick->LOAD = SystemCoreClock /1000;
		SysTick->VAL = 0;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		// 在这里阻塞等待
		while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
		us -= 1000;
	}
	SysTick->LOAD = SystemCoreClock /1000 /1000 * us;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	
	while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
	SysTick->CTRL &= SysTick_CTRL_ENABLE_Pos;
	
}

/**
 *@brief 阻塞式毫秒延时函数
 *@param ms 延时的毫秒数
 */
void delay_ms(uint32_t ms)
{
	while(ms > 0)
	{	
		SysTick->LOAD = SystemCoreClock /1000;
		SysTick->VAL = 0;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		// 在这里阻塞等待
		while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
		ms--;
	}
	SysTick->CTRL &= SysTick_CTRL_ENABLE_Pos;
	
}
