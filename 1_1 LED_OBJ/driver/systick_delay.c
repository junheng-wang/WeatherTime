#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "systick_delay.h"

void delay_us(uint32_t us)
{
	while(us > 1000)
	{
		// SystemCoreClockÊÇ
		SysTick->LOAD = SystemCoreClock /1000;
		SysTick->VAL = 0;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		
		while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
		us -= 1000;
	}
	SysTick->LOAD = SystemCoreClock /1000 /1000 * us;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
	
	while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
	SysTick->CTRL &= SysTick_CTRL_ENABLE_Pos;
	
}

void delay_ms(uint32_t ms)
{
	while(ms > 0)
	{	
		SysTick->LOAD = SystemCoreClock /1000;
		SysTick->VAL = 0;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
		
		while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
		ms--;
	}
	SysTick->CTRL &= SysTick_CTRL_ENABLE_Pos;
	
}
