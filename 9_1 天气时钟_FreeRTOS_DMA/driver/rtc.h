#ifndef __RTC_H__
#define __RTC_H__


typedef struct{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t weekday;
}RtcTimeType;


void initRtc(void);
void setRtcTime(const RtcTimeType* rtc_time);
void getRtcTime(RtcTimeType* rtc_time);

#endif
