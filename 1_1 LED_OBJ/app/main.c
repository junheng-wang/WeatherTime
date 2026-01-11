#include "board.h"
// #include "systick_delay.h"
#include "delay_tim6.h"


int main(void)
{
	// 板级初始化
	initBoardLowLevel();
	initPeripheralNvic();
	
	initTim6();
		
	// 初始化LED灯
	initLed(&led1);
	initLed(&led2);
	initLed(&led3);
	
	
	closeLed(&led1);
	closeLed(&led2);
	closeLed(&led3);
	
	while(1)
	{
		openLed(&led1);
		closeLed(&led2);
		closeLed(&led3);
		delayMsUseTim6(2000);
		
		closeLed(&led1);
		openLed(&led2);
		closeLed(&led3);
		delayMsUseTim6(2000);
		
		closeLed(&led1);
		closeLed(&led2);
		openLed(&led3);
		delayMsUseTim6(2000);
		
	}
}
