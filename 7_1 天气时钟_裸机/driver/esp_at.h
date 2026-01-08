#ifndef __ESP_AT_H__
#define __ESP_AT_H__


#define WIFI_SSID   "HONOR 70 Pro"
#define WIFI_PASSWD "WjhRyWifi0803"
#define WEATHER_URL "https://api.seniverse.com/v3/weather/now.json?key=SHKobw9DhF6IzZ58u&location=beijing&language=en&unit=c"


// Wifi的信息
typedef struct{
	char ssid[64];			// Wifi名称
	char bssid[18];			// MAC地址？？？
	int channel;
	int rssi;				// 信号强度
	uint8_t connected;		// 是否连接成功
}WifiInfoType;

// 天气的信息
typedef struct{
	char city[32];
	char loaction[128];
	char weather[16];
	int weather_code;
	float temperature;
}WeatherInfoType;

// 时间的信息
typedef struct{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t weekday;
}SntpTimeType;


uint8_t initEsp32(void);
uint8_t sendCommandAndReceiveRespondMsg(const char* command, uint32_t timeout_ms);
char* getEsp32sRsponseStr(void);
uint8_t setWifiModeToStation(void);
uint8_t connectWifiUseEsp32(const char* ssid, const char* pwd, const char* mac);
uint8_t getWifiInfo(WifiInfoType* wifi_info);
uint8_t isWifiConnect(void);
const char* getUrlInfoByHttp(const char* url);
uint8_t parseWeatherInfo(const char *response, WeatherInfoType* weather_info);

uint8_t setTimeZoneAndSntpServer(void);
uint8_t getSntpTime(SntpTimeType* time_data);
uint8_t parseSntpTimeInfo(const char* response, SntpTimeType* sntp_time);


#endif
