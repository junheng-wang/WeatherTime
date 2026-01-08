#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "esp_at.h"
#include "rtc.h"
#include "aht20.h"
#include "show_page.h"

#include "system_operation_scheduling.h"


#define MILLISECONDS(x) 	(x)							// 毫秒
#define SECONDS(x)      	MILLISECONDS((x) * 1000)
#define MINUTES(x)      	SECONDS((x) * 60)
#define HOURS(x)        	MINUTES((x) * 60)
#define DAYS(x)         	HOURS((x) * 24)

#define TIME_SYNC_INTERVAL          HOURS(1)		// SNTP时间与RTC时间每隔1小时同步一次
#define WIFI_UPDATE_INTERVAL        SECONDS(5)		// Wifi每隔5s更新一次
#define TIME_UPDATE_INTERVAL        SECONDS(1)		// 时间每隔1s更新一次
#define INNER_UPDATE_INTERVAL       SECONDS(3)		// 室内温湿度每隔3s更新一次
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(1)		// 城市天气每隔1min更新一次

#define SCHEDULINGLOGIC_EVENT_TIME_SYNC			(1 << 0)
#define SCHEDULINGLOGIC_EVENT_WIFI_UPDATE		(1 << 1)
#define SCHEDULINGLOGIC_EVENT_INNER_UPDATE		(1 << 2)
#define SCHEDULINGLOGIC_EVENT_OUTDOOR_UPDATE	(1 << 3)
#define SCHEDULINGLOGIC_EVENT_ALL				(SCHEDULINGLOGIC_EVENT_TIME_SYNC   |\
												 SCHEDULINGLOGIC_EVENT_WIFI_UPDATE |\
												 SCHEDULINGLOGIC_EVENT_INNER_UPDATE|\
												 SCHEDULINGLOGIC_EVENT_OUTDOOR_UPDATE)
												 
static TaskHandle_t system_scheduling_logic_task;
static TimerHandle_t time_sync_timer;
static TimerHandle_t wifi_update_timer;
static TimerHandle_t time_update_timer;
static TimerHandle_t inner_update_timer;
static TimerHandle_t outdoor_update_timer;

/**
 *@brief 调度1：时间同步
 */
void syncSntpTimeToRtcTime(void)
{
	SntpTimeType sntp_time = { 0 };
	RtcTimeType rtc_time = { 0 };
	
	uint32_t restart_sync_delay = HOURS(1);
	
	// 首先，联网从SNTP服务器获取最新的准确时间
	if(getSntpTime(&sntp_time) == 0)
	{
		// 获取失败，则5s之后重试
		printf("[ERROR] From SNTP get the new time filed! \r\n");
		restart_sync_delay = SECONDS(5);
		goto err;
	}
	printf("[SNTP] sync time: %04u-%02u-%02u %02u:%02u:%02u (%d) \r\n",
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
	
err:
	// 修改定时器的周期
	xTimerChangePeriod(time_sync_timer, pdMS_TO_TICKS(restart_sync_delay), 0);
	// 时间同步完成后，可立即更新一次屏幕上的时间显示
	// xTaskNotify(system_scheduling_logic_task, SCHEDULINGLOGIC_EVENT_TIME_UPDATE, eSetBits);	// 给任务发送通知
}

/**
 *@brief 调度2：获取最新的WiFi信息
 */
void updateWifiInfoToScreen(void)
{
	static WifiInfoType last_wifi_info = { 0 };
	WifiInfoType cur_wifi_info = { 0 };
	
	// 修改定时器的周期
	xTimerChangePeriod(wifi_update_timer, pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL), 0);
	
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
        printf("[WIFI] connected to %s \r\n", cur_wifi_info.ssid);
        printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d \r\n",
                cur_wifi_info.ssid, cur_wifi_info.bssid, cur_wifi_info.channel, cur_wifi_info.rssi);
		// 屏幕更新显示
        updataShowWifiSsid(cur_wifi_info.ssid);
    }
    else
    {
		// 打印调试信息
        printf("[WIFI] disconnected from %s \r\n", last_wifi_info.ssid);
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
	
	// 修改定时器的周期
	xTimerChangePeriod(inner_update_timer, pdMS_TO_TICKS(INNER_UPDATE_INTERVAL), 0);
	
	// 首先，让AHT20测量最新的温湿度信息，超时时间120ms
	if(startMeasureOfAht20(100) == 0)
	{
        printf("[AHT20] wait for measurement failed \r\n");
        return;
    }
	// 然后，读取AHT20的温湿度信息
	if(readMeasureData(&cur_temperature, &cur_humidity) == 0)
	{
        printf("[AHT20] read measurement failed \r\n");
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
	
	// 修改定时器的周期
	xTimerChangePeriod(outdoor_update_timer, pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL), 0);
	
	// 获取天气信息
    const char *weather_http_response = getUrlInfoByHttp(WEATHER_URL);
    if (weather_http_response == NULL)
    {
        printf("[WEATHER] http error \r\n");
        return;
    }
	
    // 解析天气信息
    if (parseWeatherInfo(weather_http_response, &cur_weather_info) == 0)
    {
        printf("[WEATHER] parse failed \r\n");
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
    printf("[WEATHER] %s, %s, %.1f \r\n", cur_weather_info.city, cur_weather_info.weather, cur_weather_info.temperature);
}

/**
 *@brief 整个系统的运行调度逻辑
 */
void systemSchedulingLogic(void* param)
{
	uint32_t event;
	
	while(1)
	{
		// 阻塞等待接收任务通知，通常与xTaskNotifyGive()或vTaskNotifyGiveFromISR()配对使用
		event = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		if(event & SCHEDULINGLOGIC_EVENT_TIME_SYNC)
			syncSntpTimeToRtcTime();
		if(event & SCHEDULINGLOGIC_EVENT_WIFI_UPDATE)
			updateWifiInfoToScreen();
		if(event & SCHEDULINGLOGIC_EVENT_INNER_UPDATE)
			updateInnorTemperatureHumidityToScreen();
		if(event & SCHEDULINGLOGIC_EVENT_OUTDOOR_UPDATE)
			updateOutdoorWeatherToScreen();
		//if(event & SCHEDULINGLOGIC_EVENT_TIME_SYNC)
		//	syncSntpTimeToRtcTime();
	
	}
}

/**
 *@brief 几个定时器共用的回调函数
 */
static void allTimerCallback(TimerHandle_t timer)
{
	uint32_t event = (uint32_t)pvTimerGetTimerID(timer);
	// 给任务发送通知
	xTaskNotify(system_scheduling_logic_task, event, eSetBits);
}

/**
 *@brief 时间更新定时器回调函数
 *@note1 把这个单独领出来是为了能及时更新时间，不然时间的秒针转动太卡了
 */
static void updateTimeTimerCallback(TimerHandle_t timer)
{
	updateRtcTimeToScreen();
}

/**
 *@brief 创建系统运行调度逻辑
 */
void creatSystemSchedulingLogic(void)
{
	// 创建定时器
	time_update_timer = xTimerCreate("time update", pdMS_TO_TICKS(TIME_UPDATE_INTERVAL), pdTRUE, NULL, updateTimeTimerCallback);
	
	time_sync_timer = xTimerCreate("time sync", pdMS_TO_TICKS(1), pdFALSE, (void*)SCHEDULINGLOGIC_EVENT_TIME_SYNC, allTimerCallback);
	wifi_update_timer = xTimerCreate("wifi update", pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL), pdTRUE, (void*)SCHEDULINGLOGIC_EVENT_WIFI_UPDATE, allTimerCallback);
	inner_update_timer = xTimerCreate("inner update", pdMS_TO_TICKS(INNER_UPDATE_INTERVAL), pdTRUE, (void*)SCHEDULINGLOGIC_EVENT_INNER_UPDATE, allTimerCallback);
	outdoor_update_timer = xTimerCreate("outdoor update", pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL), pdTRUE, (void*)SCHEDULINGLOGIC_EVENT_OUTDOOR_UPDATE, allTimerCallback);
	
	
	xTaskCreate(systemSchedulingLogic, "System Scheduling Logic", 1024, NULL, 5, &system_scheduling_logic_task);
	
	// 给任务发送通知
	xTaskNotify(system_scheduling_logic_task, SCHEDULINGLOGIC_EVENT_ALL, eSetBits);
	
	// 启动定时器
	xTimerStart(wifi_update_timer, 0);
	xTimerStart(time_update_timer, 0);
	xTimerStart(inner_update_timer, 0);
	xTimerStart(outdoor_update_timer, 0);
}
