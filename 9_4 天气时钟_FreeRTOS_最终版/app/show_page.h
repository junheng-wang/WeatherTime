#ifndef __SHOW_PAGE_H__
#define __SHOW_PAGE_H__


#include "ui.h"
#include "image.h"
#include "st7789.h"
#include "esp_at.h"
#include "rtc.h"


void showWelcomePage(void);
void showWirelessErrorPage(void);
void showWaitWifiConnectPage(void);
void showMainPage(void);

void updataShowWifiSsid(const char* ssid);
void updataShowTime(RtcTimeType* rtc_time);
void updataShowDate(RtcTimeType* rtc_time);
void updataShowInnerTemperature(float temperature);
void updataShowInnerHumidity(float humidity);
void updataShowOutdoorCity(const char *city);
void updataShowOutdoorTemperature(float temperature);
void updataShowOutdoorWeatherIcon(const int code);


#endif
