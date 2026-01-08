#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include "esp_at.h"
#include "systick_delay.h"


#define ESP_AT_DEBUG    0


// 返回的消息类型
typedef enum{
    AT_ACK_NONE,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY,
}AtAckType;

typedef struct{
	AtAckType at_ack;
	const char* mean;
}AtAckMatchType;

static const AtAckMatchType at_ack_match[] = {
	{AT_ACK_OK, "OK\r\n"},
	{AT_ACK_ERROR, "ERROR\r\n"},
	{AT_ACK_BUSY, "busy p…\r\n"},
	{AT_ACK_READY, "ready\r\n"},
};

// 存放ESP返回的响应信息（字符串）
static char rx_buf[1024];


void initUsart2(void);
void sendDataToEsp32ByUsart2(const char* data);
AtAckType receiveAndRecognizeResponseMsgFromEsp32ByUsart2(uint32_t timeout_ms);
AtAckType matchEsp32sAck(const char* response);
uint8_t waitReadyResponseMsg(uint32_t timeout_ms);
uint8_t waitEsp32Restart(uint32_t timeout_ms);
uint8_t parseCwstateResponseToGetWifiInfo(const char* response, WifiInfoType* wifi_info);
uint8_t parseCwjapResponseToGetApInfo(const char* response, WifiInfoType* wifi_info);


/**
 *@brief 初始化配置ESP32
 *@return 1-初始化成功，0-初始化失败
 */
uint8_t initEsp32(void)
{
	// 初始化ESP32与MCU的通信引脚
	initUsart2();
	// 等待ESP32上电重启成功
	if(waitEsp32Restart(3000) == 0)
		return 0;
	
	// 首先发送一个AT命令，测试是否能成功通信
	// if (sendCommandAndReceiveRespondMsg("AT\r\n", 100) == 0)
    //      return 0;
	
	// 发送一个复位命令
	if (sendCommandAndReceiveRespondMsg("AT+RESTORE\r\n", 2000) == 0)
        return 0;
    if (waitReadyResponseMsg(5000) == 0)
        return 0;
    
    return 1;
}

/**
 *@brief USART2外设初始化
 *@note1 USART2_TX - PA2, USART2_RX - PA3
 */
void initUsart2(void)
{
	// 1. 开启时钟（board.c文件）
	// RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// 2. 设置GPIO的复用功能
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
	// 3. 配置GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// 4. 配置USART2
	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);

	// 5. 使能USART1
	USART_Cmd(USART2, ENABLE);
}

/**
 *@brief MCU通过USART2向ESP32发送数据
 *@param data 要发送的数据
 */
void sendDataToEsp32ByUsart2(const char* data)
{
	while (data && *data)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2, *data);
		data++;
	}
}

/**
 *@brief 接收ESP32返回给MCU的响应消息
 *@param timeout_ms 超时退出时间，单位：毫秒
 *@return ESP32返回的响应消息类型
 */
AtAckType receiveAndRecognizeResponseMsgFromEsp32ByUsart2(uint32_t timeout_ms)
{
	uint32_t rx_len = 0;
    const char* line = rx_buf;
	rx_buf[0] = '\0';
	
	uint64_t start_ms = getPassedTimeUseMsUnit();
    while (rx_len < sizeof(rx_buf) - 1)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)
        {
            if(getPassedTimeUseMsUnit() - start_ms >= timeout_ms)
                return AT_ACK_NONE;	// 超时退出
        }
        rx_buf[rx_len++] = USART_ReceiveData(USART2);
        rx_buf[rx_len] = '\0';
        if(rx_buf[rx_len-1] == '\n')
		{
			AtAckType at_ack = matchEsp32sAck(line);
			if(at_ack != AT_ACK_NONE)
				return at_ack;
			
			// 指针偏移
			line = rx_buf + rx_len;
		}
    }
	return AT_ACK_NONE;
}

/**
 *@brief 匹配ESP32响应的消息类型
 *@param response ESP32响应的消息字符串
 *@return ESP32返回的响应消息类型
 */
AtAckType matchEsp32sAck(const char* response)
{
	uint8_t size = sizeof(at_ack_match)/sizeof(at_ack_match[0]);
	for(uint8_t i=0; i<size; i++)
	{
		if(strcmp(response, at_ack_match[i].mean) == 0)
			return at_ack_match[i].at_ack;
	}
	return AT_ACK_NONE;
}

/**
 *@brief MCU向ESP32发送AT指令并接收ESP32返回的消息
 *@param command要发送的AT指令
 *@param timeout_ms 接收不到消息的超时退出时间
 *@return 1-成功发送和接收，0-失败
 */
uint8_t sendCommandAndReceiveRespondMsg(const char* command, uint32_t timeout_ms)
{
	#if ESP_AT_DEBUG
		printf("[DEBUG] Send: %s\n", command);
	#endif
	
	// 发送AT指令
	sendDataToEsp32ByUsart2(command);
	// 接收响应消息
	AtAckType at_ack = receiveAndRecognizeResponseMsgFromEsp32ByUsart2(timeout_ms);
	
	#if ESP_AT_DEBUG
		printf("[DEBUG] Response:\n%s\n", rxbuf);
	#endif
	
	return at_ack == AT_ACK_OK;
}

/**
 *@brief 等待ESP32返回ready消息
 *@param timeout_ms 超时退出时间，单位：毫秒
 *@return 1-ESP32返回ready消息，0-错误
 */
uint8_t waitReadyResponseMsg(uint32_t timeout_ms)
{
	return receiveAndRecognizeResponseMsgFromEsp32ByUsart2(timeout_ms) == AT_ACK_READY;
}

/**
 *@brief 直接获取ESP32返回的响应消息字符串
 */
char* getEsp32sRsponseStr(void)
{
	return rx_buf;
}

/**
 *@brief 等待ESP32重启完成
 *@note1 因为ESP32掉电后需要一定时间才能重启成功
 *@note2 判断成功重启的标志——循环发送AT命令，直至超时
 *@param timeout_ms 超时退出时间，单位：毫秒
 *@return 1-重启成功，0-重启失败
 */
uint8_t waitEsp32Restart(uint32_t timeout_ms)
{
	while(timeout_ms > 0)
	{
		delayMsUseSystick(10);
		if(sendCommandAndReceiveRespondMsg("AT\r\n", 100) == 1)
			return 1;		
		timeout_ms -= 10;
	}
	return 0;
}

/**
 *@brief 设置 Wi-Fi 模式为 station
 *@note1 使用ESP32连接Wifi之前，必须先进行此步骤
 *@return 1-WiFi模式设置成功，0-WiFi模式设置失败
 */
uint8_t setWifiModeToStation(void)
{
	// 设置 Wi-Fi 模式为 station
	return sendCommandAndReceiveRespondMsg("AT+CWMODE=1\r\n", 2000);
}

/**
 *@brief 使用ESP32连接Wifi
 *@parma ssid Wifi名称
 *@parma pwd 登录Wifi的密码
 *@parma mac Mac地址
 *@return 1-连接成功，0-连接失败
 */
uint8_t connectWifiUseEsp32(const char* ssid, const char* pwd, const char* mac)
{
	// 参数检查
	if (ssid == NULL || pwd == NULL)
        return 0;
	// 将传入参数配置为Wifi连接命令
	char command[128];
    uint8_t len = snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
	if(mac)
		snprintf(command+len, sizeof(command)-len, "\"%s\"\r\n", mac);
	else
		snprintf(command+len, sizeof(command)-len, "\r\n");
	
	// 发送配置好的Wifi连接命令，超时时间5s
	return sendCommandAndReceiveRespondMsg(command, 5000);
}

/**
 *@brief 解析AT+CWSTATE?返回的Wifi信息
 *@param response ESP32返回的消息字符串
 *@param wifi_info 存放wifi信息的结构体
 *@return 1-解析成功，0-解析失败
 */
uint8_t parseCwstateResponseToGetWifiInfo(const char* response, WifiInfoType* wifi_info)
{
	// 入参合法性检查
    if (response == NULL)
		return 0;
	// 找到 "+CWSTATE:" 的起始位置（跳过前面的AT指令和换行）
    const char* target_start = strstr(response, "+CWSTATE:");	// strstr函数会返回"+CWSTATE:"在response的首次出现的地址
	if (target_start == NULL)
		return 0;
	// 核心解析
	int wifi_state;
	if(sscanf(target_start, "+CWSTATE:%u,\"%63[^\"]\"", &wifi_state, wifi_info->ssid) != 2)
		return 0;
	// 只有当前 Wi-Fi 状态的数字编号为2，才表示已经连接上 AP，并已经获取到 IPv4 地址
	// wifi_info->connected置1，表示已连接
	wifi_info->connected = (wifi_state == 2);
	
	return 1;
}

/**
 *@brief 解析AT+CWJAP?返回的AP(路由器)信息
 *@param response ESP32返回的消息字符串
 *@param wifi_info 存放wifi信息的结构体
 *@return 1-解析成功，0-解析失败
 */
uint8_t parseCwjapResponseToGetApInfo(const char* response, WifiInfoType* wifi_info)
{
	// 入参合法性检查
    if (response == NULL)
		return 0;
	// 找到 "+CWSTATE:" 的起始位置（跳过前面的AT指令和换行）
    const char* target_start = strstr(response, "+CWJAP:");	// strstr函数会返回"+CWSTATE:"在response的首次出现的地址
	if (target_start == NULL)
		return 0;
	// 核心解析
	if (sscanf(target_start, "+CWJAP:\"%63[^\"]\",\"%17[^\"]\",%d,%d", wifi_info->ssid, wifi_info->bssid, &wifi_info->channel, &wifi_info->rssi) != 4)
		return 0;
	
	return 1;
}
/**
 *@breif 获取当前的Wifi信息和AP(路由器)信息
 */
uint8_t getWifiInfo(WifiInfoType* wifi_info)
{
	// 查询 ESP32-C3 设备的 Wi-Fi 状态和 Wi-Fi 信息
	if(sendCommandAndReceiveRespondMsg("AT+CWSTATE?\r\n", 2000) == 0)
		return 0;	// 查询失败
	// 查询成功，则需解析ESP32返回的 Wifi 信息
	if(parseCwstateResponseToGetWifiInfo(getEsp32sRsponseStr(), wifi_info) == 0)
		return 0;	// 解析失败
	
	// 查询与 ESP32-C3 Station 连接的 AP 信息
	if(sendCommandAndReceiveRespondMsg("AT+CWJAP?\r\n", 2000) == 0)
		return 0;	// 查询失败
	// 查询成功则需解析ESP32返回的 AP 信息
	if(parseCwjapResponseToGetApInfo(getEsp32sRsponseStr(), wifi_info) == 0)
		return 0;	// 解析失败
	
	return 1;
}

/**
 *@brief 查询Wi-Fi是否连接
 *@return 1-WiFi连接成功，0-WiFi连接失败
 */
uint8_t isWifiConnect(void)
{
	WifiInfoType wifi_info = {0};
	if(getWifiInfo(&wifi_info) == 0)
		return 0;
	
	return wifi_info.connected == 1;	
}

/**
 *@brief 连接上Wifi后，就可以请求网页内容了
 *@param url 网站连接
 *@return 网页信息字符串（此字符串后面需要进行解析，以获得具体的天气信息）
 */
const char* getUrlInfoByHttp(const char* url)
{
	// 仅仅是一个小技巧：借用rx_buf，减少内存开销
	char* txbuf = rx_buf;
    snprintf(txbuf, sizeof(rx_buf), "AT+HTTPCLIENT=2,1,\"%s\",,,2\r\n", url);
    uint8_t ret = sendCommandAndReceiveRespondMsg(txbuf, 5000);
	
    return ret==1 ? getEsp32sRsponseStr() : NULL;
}

/**
 *@brief 使用sscanf函数解析获取的天气信息
 *@parma response ESP32通过联网查询心知天气网页返回的信息
 *@parma info 是一个传出参数，保存解析到的天气数据
 *@return 1-解析成功，0-解析失败
 */
uint8_t parseWeatherInfo(const char *response, WeatherInfoType* weather_info)
{
	response = strstr(response, "\"results\":");
	if (response == NULL)
		return 0;
	
	const char *location_response = strstr(response, "\"location\":");
	if (location_response == NULL)
		return 0;
	
	const char *loaction_name_response = strstr(location_response, "\"name\":");
	if (loaction_name_response)
		sscanf(loaction_name_response, "\"name\": \"%31[^\"]\"", weather_info->city);
	
	const char *loaction_path_response = strstr(location_response, "\"path\":");
	if (loaction_path_response)
		sscanf(loaction_path_response, "\"path\": \"%128[^\"]\"", weather_info->loaction);
	
	const char *now_response = strstr(response, "\"now\":");
	if (now_response == NULL)
		return 0;
	
	const char *now_text_response = strstr(now_response, "\"text\":");
	if (now_text_response)
		sscanf(now_text_response, "\"text\": \"%15[^\"]\"", weather_info->weather);
	
	const char *now_code_response = strstr(now_response, "\"code\":");
	if (now_code_response)
		sscanf(now_code_response, "\"code\": \"%d\"", &weather_info->weather_code);
	
	char temperature_str[16] = { 0 };
	const char *now_temperature_response = strstr(now_response, "\"temperature\":");
	if (now_temperature_response)
	{
		if (sscanf(now_temperature_response, "\"temperature\": \"%15[^\"]\"", temperature_str) == 1)
			weather_info->temperature = atof(temperature_str);
	}
	return 1;
}

/**
 *@brief 设置时区和 SNTP 服务器
 *@note1 获取SNTP时间之前，必须先进行此步骤
 *@return 1-SNTP设置成功，0-SNTP设置失败
 */
uint8_t setTimeZoneAndSntpServer(void)
{
	// 设置时区和 SNTP 服务器（超时时间2s）
	return sendCommandAndReceiveRespondMsg("AT+CIPSNTPCFG=1,8\r\n", 2000);
}

/**
 *@brief 从 SNTP 服务器上获取时间
 *@note1 获取SNTP时间之前，必须先进行此步骤
 *@return 1-时间获取成功，0-时间获取失败
 */
uint8_t getSntpTime(SntpTimeType* time_data)
{
	// 1. 向ESP32发送获取时间的AT命令
	if(sendCommandAndReceiveRespondMsg("AT+CIPSNTPTIME?\r\n", 2000) == 0)
		return 0;
	
	// 2. 解析ESP传回的SNTP时间信息
	if (parseSntpTimeInfo(getEsp32sRsponseStr(), time_data) == 0)
		return 0;
	return 1;	
}

/**
 *@brief 辅助函数：将英文月份（Jan/Feb等）转为数字（1-12）
 *@return 月份对应的数字
 */
static uint8_t monthStrToNum(const char *month_str)
{
    if (strcmp(month_str, "Jan") == 0) return 1;
    if (strcmp(month_str, "Feb") == 0) return 2;
    if (strcmp(month_str, "Mar") == 0) return 3;
    if (strcmp(month_str, "Apr") == 0) return 4;
    if (strcmp(month_str, "May") == 0) return 5;
    if (strcmp(month_str, "Jun") == 0) return 6;
    if (strcmp(month_str, "Jul") == 0) return 7;
    if (strcmp(month_str, "Aug") == 0) return 8;
    if (strcmp(month_str, "Sep") == 0) return 9;
    if (strcmp(month_str, "Oct") == 0) return 10;
    if (strcmp(month_str, "Nov") == 0) return 11;
    if (strcmp(month_str, "Dec") == 0) return 12;
    return 0; // 无效月份
}
/**
 *@brief 辅助函数：将英文星期（Mon/Tue等）转为数字（1=周一，7=周日）
 *@return 星期对应的数字
 */
static uint8_t weekdayStrToNum(const char *weekday_str)
{
    if (strcmp(weekday_str, "Mon") == 0) return 1;
    if (strcmp(weekday_str, "Tue") == 0) return 2;
    if (strcmp(weekday_str, "Wed") == 0) return 3;
    if (strcmp(weekday_str, "Thu") == 0) return 4;
    if (strcmp(weekday_str, "Fri") == 0) return 5;
    if (strcmp(weekday_str, "Sat") == 0) return 6;
    if (strcmp(weekday_str, "Sun") == 0) return 7;
    return 0; // 无效星期
}
/**
 *@brief 使用sscanf函数解析获取的天气信息
 *@parma response ESP32通过联网查询的SNTP时间信息
 *@parma sntp_time 是一个传出参数，保存解析到的天气数据
 *@return 1-解析成功，0-解析失败
 */
uint8_t parseSntpTimeInfo(const char* response, SntpTimeType* sntp_time)
{
	// 1. 入参合法性检查
    if (response == NULL || sntp_time == NULL)
        return 0;
	// 2. 初始化结构体，避免脏数据
    memset(sntp_time, 0, sizeof(SntpTimeType));
	// 3. 定位时间行起始位置（跳过前面的AT指令，找到"+CIPSNTPTIME:"）
    const char *time_line = strstr(response, "+CIPSNTPTIME:");
    if (time_line == NULL)
        return 0; // 未找到时间行前缀
    time_line += strlen("+CIPSNTPTIME:"); // 跳过前缀，定位到时间字符串起始
	
	// 4. 定义临时变量存储解析的英文月份/星期
    char weekday_str[4] = {0};  // 存储Mon/Tue等（3字符+结束符）
    char month_str[4] = {0};    // 存储Jan/Feb等（3字符+结束符）
    uint16_t year = 0;
    uint8_t day = 0, hour = 0, minute = 0, second = 0;
	
	// 5. 核心解析：按"Mon Jan  5 20:24:21 2026"格式提取字段
    // 格式串说明：
    // %3s: 读取3字符（星期，如Mon）；%3s: 读取3字符（月份，如Jan）；
    // %hhu: 读取日（如5）；%hhu:%hhu:%hhu: 读取时:分:秒（如20:24:21）；%hu: 读取年（如2026）
	int ret = sscanf(time_line, "%3s %3s %hhu %hhu:%hhu:%hhu %hu",
                     weekday_str, month_str, &day, &hour, &minute, &second, &year);
	// 检查是否成功解析7个字段
    if (ret != 7) 
        return 0;
	// 6. 转换英文月份/星期为数字，并赋值到结构体
    sntp_time->weekday = weekdayStrToNum(weekday_str);
    sntp_time->month = monthStrToNum(month_str);
    // 校验月份和星期的有效性
    if (sntp_time->month == 0 || sntp_time->weekday == 0)
        return 0;

    // 7. 赋值其他字段
    sntp_time->year = year;
    sntp_time->day = day;
    sntp_time->hour = hour;
    sntp_time->minute = minute;
    sntp_time->second = second;

	return 1;
}


