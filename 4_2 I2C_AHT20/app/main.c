#include <stdio.h>

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "aht20.h"
#include "usart1.h"
#include "systick_delay.h"

int main(void)
{
	float temperature;
	float humidity;
	
	// 1. 初始化USART1
	initUsart1();
	// 2. 初始化AHT20
	if(initAht20() == 0)
		printf("[ERROR] aht20 init failed!!!\r\n");
	
	while(1)
	{
		// 启动测量
		if (startMeasureOfAht20(255) == 0)
        {
            printf("[ERROR] aht20 start measurement failed!!!\r\n");
            continue;
        }
		
		// 读取数据
		if(readMeasureData(&temperature, &humidity) == 0)
		{
			printf("[ERROR] aht20 read measurement failed!!!\r\n");
			continue;
		}
		printf("temperature: %.2f, humidity:%.2f\r\n", temperature, humidity);
		delay_ms(1000);
	}
}