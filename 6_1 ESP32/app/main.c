#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include "systick_delay.h"
#include "usart1.h"
#include "esp_at.h"

WeatherInfoType WeatherInfo;
SntpTimeType sntp_time;

int main(void)
{
	delayOneMsUseInterruptModeInit();
	initUsart1();
	
	if (initEsp32() == 0)
    {
        printf("[AT] init failed\r\n");
        goto err;
    }
    printf("[AT] inited\r\n");
    
    if (setWifiModeToStation() == 0)
    {
        printf("[WIFI] init failed\r\n");
        goto err;
    }
    printf("[WIFI] inited\r\n");
    
    if (connectWifiUseEsp32(WIFI_SSID, WIFI_PASSWD, NULL) == 0)
    {
        printf("[WIFI] connect failed\r\n");
        goto err;
    }
    printf("[WIFI] connecting\r\n");
	
	if(setTimeZoneAndSntpServer() == 0)
	{
        printf("[SNTP] init failed\r\n");
        goto err;
    }
	
	while(1)
	{
//		const char* weater = getUrlInfoByHttp(WEATHER_URL);
//		if(weater == NULL)
//		{
//			printf("[WEATHER] WEATHER INFO get failed\r\n");
//			delayMsUseSystick(1000);
//			continue;
//		}
//			
//		if(parseWeatherInfo(weater, &WeatherInfo) == 0)
//		{
//			printf("[WEATHER] WEATHER INFO parse failed\r\n");
//			delayMsUseSystick(1000);
//			continue;
//		}
//		printf("city: %s \r\n", WeatherInfo.city);
//		printf("temperature: %f \r\n", WeatherInfo.temperature);
		
		getSntpTime(&sntp_time);
		if(parseSntpTimeInfo(getEsp32sRsponseStr(), &sntp_time))
		{
			printf("%d-%d-%d %d:%d:%d  weekday: %d\r\n", sntp_time.year, sntp_time.month, sntp_time.day, sntp_time.hour, sntp_time.minute, sntp_time.second, sntp_time.weekday);
		}
		delayMsUseSystick(1500);
	}
	
err:
    while (1)
    {
		printf("JunHeng\r\n");
        printf("AT Error\r\n");
        delayMsUseSystick(1000);
    }
}

