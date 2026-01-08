#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "rtc.h"


/**
 *@brief 初始化RTC外设
 */
void initRtc(void)
{
	/* 开启时钟这一块，参见board.c文件
	// 1. 启用PWR和BKP的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	// 2. 后备域解锁，允许访问RTC和后备寄存器
    PWR_BackupAccessCmd(ENABLE);
	// 3. 复位备份域，清除之前的RTC配置
    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);
	// 4. 使能LSE——外部32.768kHz晶振
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	// 等待LSE就绪
	// 5. RTC时钟源配置成LSE（外部32.768K）
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	// 6. RTC时钟使能
    RCC_RTCCLKCmd(ENABLE);
	// 7. 开启后需要等待APB1时钟与RTC数据同步（影子寄存器的读写），才能读写寄存器
    RTC_WaitForSynchro();
	*/
	
	// 8. 配置 RTC 初始化结构体
	RTC_InitTypeDef RTC_InitStruct;
	RTC_StructInit(&RTC_InitStruct);
    RTC_InitStruct.RTC_HourFormat = RTC_HourFormat_24;      // 24小时制
    RTC_InitStruct.RTC_AsynchPrediv = 0x7F;                // 异步预分频 = 127
    RTC_InitStruct.RTC_SynchPrediv = 0xFF;                 // 同步预分频 = 255
    // 总分频 = (AsynchPrediv + 1) * (SynchPrediv + 1) = 128 * 256 = 32768 → 1Hz
    RTC_Init(&RTC_InitStruct);
	
}

/**
 *@brief 设置RTC时间
 *@param rtc_time rtc时间结构体，保存时间信息
 */
void setRtcTime(const RtcTimeType* rtc_time)
{
	RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
	
	RTC_DateStructInit(&RTC_DateStruct);
    RTC_TimeStructInit(&RTC_TimeStruct);
	
	
	// 配置时间（使用 BIN 格式，直接传入二进制值）
    RTC_TimeStruct.RTC_Hours   = rtc_time->hour;     // 0~23
    RTC_TimeStruct.RTC_Minutes = rtc_time->minute;   // 0~59
    RTC_TimeStruct.RTC_Seconds = rtc_time->second;   // 0~59

    // 配置日期
    RTC_DateStruct.RTC_Year    = rtc_time->year - 2000; // SPL 要求年份是 0~99（2000年起）
    RTC_DateStruct.RTC_Month   = rtc_time->month;       // 1~12
    RTC_DateStruct.RTC_Date    = rtc_time->day;         // 1~31
    RTC_DateStruct.RTC_WeekDay = rtc_time->weekday;     // 1=周一, 7=周日（STM32 定义）

    // 写入 RTC
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
}

/**
 *@brief 读取RTC时间
 *@param rtc_time rtc时间结构体，保存时间信息
 */
void getRtcTime(RtcTimeType* rtc_time)
{
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	rtc_time->hour   = RTC_TimeStruct.RTC_Hours;
	rtc_time->minute = RTC_TimeStruct.RTC_Minutes;
	rtc_time->second = RTC_TimeStruct.RTC_Seconds;
	
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	rtc_time->year    = RTC_DateStruct.RTC_Year + 2000;
	rtc_time->month   = RTC_DateStruct.RTC_Month;
	rtc_time->day     = RTC_DateStruct.RTC_Date;
	rtc_time->weekday = RTC_DateStruct.RTC_WeekDay;    // 1=周一, ..., 7=周日
}
