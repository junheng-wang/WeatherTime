#include <stdio.h>
#include <string.h>
#include "system_operation_scheduling.h"
#include "systick_delay.h"
#include "esp_at.h"
#include "rtc.h"
#include "aht20.h"
#include "show_page.h"


#define MILLISECONDS(x) 	(x)							// 毫秒
#define SECONDS(x)      	MILLISECONDS((x) * 1000)
#define MINUTES(x)      	SECONDS((x) * 60)
#define HOURS(x)        	MINUTES((x) * 60)
#define DAYS(x)         	HOURS((x) * 24)

#define TIME_SYNC_INTERVAL          DAYS(1)			// SNTP时间与RTC时间每隔1天同步一次
#define WIFI_UPDATE_INTERVAL        SECONDS(5)		// Wifi每隔5s更新一次
#define TIME_UPDATE_INTERVAL        SECONDS(1)		// 时间每隔1s更新一次
#define INNER_UPDATE_INTERVAL       SECONDS(3)		// 室内温湿度每隔3s更新一次
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(1)		// 城市天气每隔1min更新一次


// 局部变量：将SNTP获取的时间更新到RTC外设的时间间隔
static uint32_t time_sync_delay = 0;
// 局部变量：更新在屏幕上显示WiFi信息的时间间隔
static uint32_t wifi_update_delay = 0;
// 局部变量：更新在屏幕上显示时间信息的时间间隔
static uint32_t time_update_delay = 0;
// 局部变量：更新在屏幕上显示室内温湿度信息的时间间隔
static uint32_t inner_update_delay = 0;
// 局部变量：更新在屏幕上显示室外天气信息的时间间隔
static uint32_t outdoor_update_delay = 0;

/**
 *@brief Systic的回调函数
 */
static void periodicCallbackForSystick(void)
{
    if (time_sync_delay > 0)        time_sync_delay--;
    if (wifi_update_delay > 0)      wifi_update_delay--;
    if (time_update_delay > 0)      time_update_delay--;
    if (inner_update_delay > 0)     inner_update_delay--;
    if (outdoor_update_delay > 0)   outdoor_update_delay--;
}

/**
 *@brief 注册Systic的回调函数，这样在systick中断中就可以调用了
 */
void forSystickRegisterCallback(void)
{
	registerSystickPeriodicCallback(periodicCallbackForSystick);
}

/**
 *@brief 调度1：时间同步
 */
void syncSntpTimeToRtcTime(void)
{
	SntpTimeType sntp_time = { 0 };
	RtcTimeType rtc_time = { 0 };
	
	// 首先，联网从SNTP服务器获取最新的准确时间
	if(getSntpTime(&sntp_time) == 0)
	{
		// 获取失败，则5s之后重试
		printf("[ERROR] From SNTP get the new time filed! \r\n");
		time_sync_delay = SECONDS(5);
		return;
	}
	printf("[SNTP] sync time: %04u-%02u-%02u %02u:%02u:%02u (%d)\n",
        sntp_time.year, sntp_time.month, sntp_time.day,
        sntp_time.hour, sntp_time.minute, sntp_time.second, sntp_time.weekday);
	
	// 获取SNTP时间成功后，将时间设置到RTC时间结构体中
	rtc_time.year    = sntp_time.year;
	rtc_time.month   = sntp_time.month;
	rtc_time.day     = sntp_time.day;
	rtc_time.hour    = sntp_time.hour;
	rtc_time.minute  = sntp_time.minute;
	rtc_time.second  = sntp_time.second;
	rtc_time.weekday = sntp_time.weekday;	
	
	// 完成同步：将SNTP时间设置到RTC中
	setRtcTime(&rtc_time);
	
	// 时间同步完成后，可立即更新一次屏幕上的时间显示
	// time_update_delay = 0;
}

/**
 *@brief 调度2：获取最新的WiFi信息
 */
void updateWifiInfoToScreen(void)
{
	static WifiInfoType last_wifi_info = { 0 };
	WifiInfoType cur_wifi_info = { 0 };
	
	// 首先，联网获取WiFi信息
	if(getWifiInfo(&cur_wifi_info) == 0)
	{
		printf("[ERROR] Get the new WiFi information filed! \r\n");
		// 屏幕更新显示Wifi信号丢失
        updataShowWifiSsid("wifi lost"); 
        return;
	}
	// 若本次获取的Wiif信息和上一次的Wifi信息没有变化，则不做任何处理
	if (memcmp(&cur_wifi_info, &last_wifi_info, sizeof(WifiInfoType)) == 0)
        return;
	
	// 刷新last_wifi_info的变量内容
	memcpy(&last_wifi_info, &cur_wifi_info, sizeof(WifiInfoType));
	
	// 在屏幕上显示更新的Wifi信息
	if (cur_wifi_info.connected)
    {
		// 打印调试信息
        printf("[WIFI] connected to %s\n", cur_wifi_info.ssid);
        printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                cur_wifi_info.ssid, cur_wifi_info.bssid, cur_wifi_info.channel, cur_wifi_info.rssi);
		// 屏幕更新显示
        updataShowWifiSsid(cur_wifi_info.ssid);
    }
    else
    {
		// 打印调试信息
        printf("[WIFI] disconnected from %s\n", last_wifi_info.ssid);
		// 屏幕更新显示Wifi信号丢失
        updataShowWifiSsid("wifi lost");
    }
}

/**
 *@brief 调度3：从RTC获取时间更新到屏幕上
 */
void updateRtcTimeToScreen(void)
{
	static RtcTimeType last_rtc_time = { 0 };
	RtcTimeType cur_rtc_time = { 0 };
	// 首先，从RTC获取时间
	getRtcTime(&cur_rtc_time);
	// 然后，更新到屏幕上
	updataShowTime(&cur_rtc_time);
	
	// 对于日期的更新，则可通过判断的方式，毕竟日期一天才变化一次
	if(last_rtc_time.year == cur_rtc_time.year && last_rtc_time.month == cur_rtc_time.month && last_rtc_time.day == cur_rtc_time.day)
		return;
	else
		updataShowDate(&cur_rtc_time);
	
	last_rtc_time = cur_rtc_time;
}

/**
 *@brief 调度4：从AHT20获取室内温湿度，并更新到屏幕上
 */
void updateInnorTemperatureHumidityToScreen(void)
{
	static float last_temperature;
	static float last_humidity;
	
	float cur_temperature;
	float cur_humidity;
	
	// 首先，让AHT20测量最新的温湿度信息，超时时间120ms
	if(startMeasureOfAht20(100) == 0)
	{
        printf("[AHT20] wait for measurement failed\n");
        return;
    }
	// 然后，读取AHT20的温湿度信息
	if(readMeasureData(&cur_temperature, &cur_humidity) == 0)
	{
        printf("[AHT20] read measurement failed\n");
        return;
    }
	
	// 更新显示温湿度信息到屏幕
	if(last_temperature != cur_temperature)
	{
		updataShowInnerTemperature(cur_temperature);
		last_temperature = cur_temperature;
	}
	if(last_humidity != cur_humidity)
	{
		updataShowInnerHumidity(cur_humidity);
		last_humidity = cur_humidity;
	}
}

/**
 *@brief 调度5：从心知天气获取室外指定城市的天气，并更新到屏幕上
 */
void updateOutdoorWeatherToScreen(void)
{
	static WeatherInfoType last_weather_info;
	WeatherInfoType cur_weather_info;
	
	// 获取天气信息
    const char *weather_http_response = getUrlInfoByHttp(WEATHER_URL);
    if (weather_http_response == NULL)
    {
        printf("[WEATHER] http error\n");
        return;
    }
	
    // 解析天气信息
    if (parseWeatherInfo(weather_http_response, &cur_weather_info) == 0)
    {
        printf("[WEATHER] parse failed\n");
        return;
    }
	
    // 若天气信息没有变化，则不做任何处理
    if (memcmp(&last_weather_info, &cur_weather_info, sizeof(WeatherInfoType)) == 0)
        return;
	
    // 屏幕更新显示
	updataShowOutdoorTemperature(cur_weather_info.temperature);
    updataShowOutdoorWeatherIcon(cur_weather_info.weather_code);
	
	// 更新上一次的测量数据
    memcpy(&last_weather_info, &cur_weather_info, sizeof(WeatherInfoType));
	
	// 打印天气调试信息
    printf("[WEATHER] %s, %s, %.1f\n", cur_weather_info.city, cur_weather_info.weather, cur_weather_info.temperature);
}

/**
 *@brief 整个系统的运行调度逻辑
 */
void systemSchedulingLogic(void)
{
	if (time_sync_delay <= 0)
	{
		time_sync_delay = TIME_SYNC_INTERVAL;
		syncSntpTimeToRtcTime();
	}
	
    if (wifi_update_delay <= 0)
	{
		wifi_update_delay = WIFI_UPDATE_INTERVAL;
		updateWifiInfoToScreen();
	}
	
    if (time_update_delay <= 0)
	{
		time_update_delay = TIME_UPDATE_INTERVAL;
		updateRtcTimeToScreen();
	}
	if (inner_update_delay <= 0)
	{
		inner_update_delay = INNER_UPDATE_INTERVAL;
		updateInnorTemperatureHumidityToScreen();
	}
    if (outdoor_update_delay <= 0)
	{
		outdoor_update_delay = OUTDOOR_UPDATE_INTERVAL;
		updateOutdoorWeatherToScreen();
	}	
	
}
