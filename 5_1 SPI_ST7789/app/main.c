#include <stdint.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "st7789.h"


int main(void)
{
	initSt7798();
	setBackGroundLed(1);
//	fillColorForSt7789(0, 0, 79, 319, mkcolor(255, 0, 0));
//	fillColorForSt7789(80, 0, 159, 319, mkcolor(0, 255, 0));
//	fillColorForSt7789(160, 0, 239, 319, mkcolor(0, 0, 255));
//	fillColorForSt7789(0, 300, 239, 319, mkcolor(0, 0, 0));
//	showString(10, 10, "Hello Junheng", mkcolor(255, 255, 0), mkcolor(0, 0, 0), &font32);
//	showString(10, 100, "梅花嵌入式梅花嵌入式天气时钟", mkcolor(20, 99, 180), mkcolor(255, 255, 255), &font32);
	showImage(0, 0, &image_tv);
	showString(10, 10, "Hello Junheng", mkcolor(255, 255, 0), mkcolor(0, 0, 0), &font32);
	showString(10, 100, "梅花嵌入式梅花嵌入式天气时钟", mkcolor(20, 99, 180), mkcolor(255, 255, 255), &font32);
	
	while(1)
	{
		
	}
}
