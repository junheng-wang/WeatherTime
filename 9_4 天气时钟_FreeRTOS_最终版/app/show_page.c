#include "show_page.h"


void updataShowWifiSsid(const char* ssid);
void updataShowTime(RtcTimeType* rtc_time);
void updataShowDate(RtcTimeType* rtc_time);
void updataShowInnerTemperature(float temperature);
void updataShowInnerHumidity(float humidity);
void updataShowOutdoorCity(const char *city);
void updataShowOutdoorTemperature(float temperature);
void updataShowOutdoorWeatherIcon(const int code);


// 定义三块背景色的局部变量
static const uint16_t color_bg_time = mkcolor(248, 248, 248);
static const uint16_t color_bg_inner = mkcolor(136, 217, 234);
static const uint16_t color_bg_outdoor = mkcolor(254, 135, 75);


/**
 *@brief 开机显示页面
 */
void showWelcomePage(void)
{
	const uint16_t color_bg = mkcolor(0, 0, 0);
	// 清屏
	uiFillColorParamSendToQueue(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, color_bg);
	// 显示梅花图案
	uiShowImageParamSendToQueue(30, 10, &img_meihua);
	// 显示文字
	uiShowStringParamSendToQueue(40, 205, "梅花嵌入式", mkcolor(237, 128, 147), color_bg, &font32_maple_bold);
    uiShowStringParamSendToQueue(56, 233, "天气时钟", mkcolor(86, 165, 255), color_bg, &font32_maple_bold);
    uiShowStringParamSendToQueue(60, 285, "loading...", mkcolor(255, 255, 255), color_bg, &font24_maple_bold);
}

/**
 *@brief 联网模块ESP32错误界面
 */
void showWirelessErrorPage(void)
{
//	uiFillColorParamSendToQueue(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, mkcolor(0, 0, 0));	// 先将页面全部刷新为黑色
//	uiShowImageParamSendToQueue(40, 37, &img_error);							// 显示ESP32的WIFI或SNTP初始化失败警醒图片
//	uiShowStringParamSendToQueue(5, 240, "Wireless Init Filed", mkcolor(255, 255, 0), mkcolor(0, 0, 0), &font24_maple_bold);
	fillColorForSt7789(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, mkcolor(0, 0, 0));	// 先将页面全部刷新为黑色
	showImage(40, 37, &img_error);							// 显示ESP32的WIFI或SNTP初始化失败警醒图片
	showString(5, 240, "Wireless Init Filed", mkcolor(255, 255, 0), mkcolor(0, 0, 0), &font24_maple_bold);
}

/**
 *@brief 等待WiFi连接页面
 */
void showWaitWifiConnectPage(void)
{
	uiFillColorParamSendToQueue(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, mkcolor(0, 0, 0));	// 先将页面全部刷新为黑色
	uiShowImageParamSendToQueue(30, 37, &img_wifi);							// 显示ESP32的WIFI或SNTP尝试连接Wifi
	uiShowStringParamSendToQueue(85, 210, "WIFI", mkcolor(0, 255, 255), mkcolor(0, 0, 0), &font32_maple_bold);
	uiShowStringParamSendToQueue(30, 250, "[HONOR 70 Pro]", mkcolor(0, 255, 255), mkcolor(0, 0, 0), &font24_maple_bold);
	uiShowStringParamSendToQueue(75, 280, "连接中", mkcolor(0, 0, 255), mkcolor(0, 0, 0), &font24_maple_bold);
}

/**
 *@brief 主页面，即三个背景矩形框
 */
void showMainPage(void)
{
	uiFillColorParamSendToQueue(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1, mkcolor(0, 0, 0));	// 先将页面全部刷新为黑色
	
	uiFillColorParamSendToQueue(15, 15, 224, 154, color_bg_time);	// 上半部分绘制显示时间的白色框
	uiShowImageParamSendToQueue(23, 20, &icon_wifi);		// Wifi图标
	updataShowWifiSsid(WIFI_SSID);
	uiShowStringParamSendToQueue(25, 42, "--:--", mkcolor(0, 0, 0), color_bg_time, &font76_maple_extrabold);			// 显示时间
	uiShowStringParamSendToQueue(35, 121, "----/--/-- ------", mkcolor(0, 0, 0), color_bg_time, &font20_maple_bold);	// 显示日期
	
	uiFillColorParamSendToQueue(15, 165, 114, 304, color_bg_inner);	// 左下部分绘制芯片所处环境的温湿度
	uiShowStringParamSendToQueue(19, 170, "室内环境", mkcolor(0, 0, 0), color_bg_inner, &font24_maple_semibold);
    uiShowStringParamSendToQueue(86, 191, "C", mkcolor(0, 0, 0), color_bg_inner, &font32_maple_bold);
	updataShowInnerTemperature(999.9f);
    uiShowStringParamSendToQueue(91, 262, "%", mkcolor(0, 0, 0),color_bg_inner, &font32_maple_bold);
	updataShowInnerHumidity(999.9f);
	
	uiFillColorParamSendToQueue(125, 165, 224, 304, color_bg_outdoor);	// 右下部分绘制选定城市的天气
	updataShowOutdoorCity("北京");
	uiShowStringParamSendToQueue(192, 189, "C", mkcolor(0, 0, 0), color_bg_outdoor, &font20_maple_bold);
	updataShowOutdoorTemperature(999.9f);
	
	uiShowImageParamSendToQueue(139, 239, &icon_wenduji);		// 温度计图标
	updataShowOutdoorWeatherIcon(1);
}

/**
 *@brief 更新显示Wifi名称
 *@note1 当wifi信息改变的时候，只刷新wifi对应的屏幕显示区域，而不是刷新整个屏幕
 *@note2 这个函数的参数 const char *ssid 是一个常量指针，这是为了保护传入的数据不被函数意外修改，确保函数只读取数据，不改变外部状态。
 *@param ssid WiFi的名称
 */
void updataShowWifiSsid(const char* ssid)
{
	char wifi_name[21];
	// 将 ssid 格式化为右对齐的 20 个字符，并存储到 str 中
    snprintf(wifi_name, sizeof(wifi_name), "%20s", ssid);
    uiShowStringParamSendToQueue(50, 23, wifi_name, mkcolor(143, 143, 143), color_bg_time, &font16_maple);
}

/**
 *@brief 更新显示时间信息
 *@note1 当时间信息改变的时候，只刷新时间对应的屏幕显示区域，而不是刷新整个屏幕
 *@param rtc_time 从RTC获取的时间信息
 */
void updataShowTime(RtcTimeType* rtc_time)
{
	char str[6];
	// 闪烁显示时间分隔符":"
    char comma = (rtc_time->second % 2 == 0) ? ':' : ' ';
    snprintf(str, sizeof(str), "%02u%c%02u", rtc_time->hour, comma, rtc_time->minute);
    uiShowStringParamSendToQueue(25, 42, str, mkcolor(0, 0, 0), color_bg_time, &font76_maple_extrabold);
}

/**
 *@brief 更新显示日期信息
 *@note1 当日期信息改变的时候，只刷新日期对应的屏幕显示区域，而不是刷新整个屏幕
 *@param rtc_time 从RTC获取的日期信息
 */
void updataShowDate(RtcTimeType* rtc_time)
{
	char str[18];
	snprintf(str, sizeof(str), "%04u/%02u/%02u 星期%s", rtc_time->year, rtc_time->month, rtc_time->day,
		rtc_time->weekday == 1 ? "一" : 
		rtc_time->weekday == 2 ? "二" :
		rtc_time->weekday == 3 ? "三" :
		rtc_time->weekday == 4 ? "四" :
		rtc_time->weekday == 5 ? "五" :
	    rtc_time->weekday == 6 ? "六" :
		rtc_time->weekday == 7 ? "天" : "X");
	uiShowStringParamSendToQueue(35, 121, str, mkcolor(0, 0, 0), color_bg_time, &font20_maple_bold);
}

/**
 *@brief 更新显示室内温度信息
 *@note1 当室内温度信息改变的时候，只刷新室内温度对应的屏幕显示区域，而不是刷新整个屏幕
 */
void updataShowInnerTemperature(float temperature)
{
    char str[3] = {'-', '-'};
    if (temperature > -10.0f && temperature <= 100.0f)
        snprintf(str, sizeof(str), "%2.0f", temperature);
	
    uiShowStringParamSendToQueue(30, 192, str, mkcolor(0, 0, 0), color_bg_inner, &font54_maple_semibold);
}

/**
 *@brief 更新显示室内湿度信息
 *@note1 当室内湿度信息改变的时候，只刷新室内湿度对应的屏幕显示区域，而不是刷新整个屏幕
 */
void updataShowInnerHumidity(float humidity)
{
    char str[3];
    if (humidity > 0.0f && humidity <= 99.99f)
        snprintf(str, sizeof(str), "%2.0f", humidity);
	
    uiShowStringParamSendToQueue(25, 239, str, mkcolor(0, 0, 0), color_bg_inner, &font64_maple_extrabold);
}

/**
 *@brief 更新显示室外城市信息
 *@note1 当室内温度信息改变的时候，只刷新室内温度对应的屏幕显示区域，而不是刷新整个屏幕
 */
void updataShowOutdoorCity(const char *city)
{
    char str[9];
    snprintf(str, sizeof(str), "%s", city);
    uiShowStringParamSendToQueue(127, 170, str, mkcolor(0, 0, 0), color_bg_outdoor, &font24_maple_semibold);
}

/**
 *@brief 更新显示室外城市对应的温度
 *@note1 当室外城市对应的温度改变的时候，只刷新室外城市对应的温度对应的屏幕显示区域，而不是刷新整个屏幕
 */
void updataShowOutdoorTemperature(float temperature)
{
    char str[3] = {'-', '-'};
    if (temperature > -10.0f && temperature <= 100.0f)
        snprintf(str, sizeof(str), "%2.0f", temperature);
    uiShowStringParamSendToQueue(135, 190, str, mkcolor(0, 0, 0), color_bg_outdoor, &font54_maple_bold);
}

/**
 *@brief 更新显示室外城市的天气图标
 *@param code 心知天气返回的天气代码，不同天气对应不同整数代码
 */
void updataShowOutdoorWeatherIcon(const int code)
{
    const ImageType* icon;
    if (code == 0 || code == 2 || code == 38)
        icon = &icon_qing;
    else if (code == 1 || code == 3)
        icon = &icon_yueliang;
    else if (code == 4 || code == 9)
        icon = &icon_yintian;
    else if (code == 5 || code == 6 || code == 7 || code == 8)
        icon = &icon_duoyun;
    else if (code == 10 || code == 13 || code == 14 || code == 15 || code == 16 || code == 17 || code == 18 || code == 19)
        icon = &icon_zhongyu;
    else if (code == 11 || code == 12)
        icon = &icon_leizhenyu;
    else if (code == 20 || code == 21 || code == 22 || code == 23 || code == 24 || code == 25)
        icon = &icon_zhongxue;
    else // 扬沙、龙卷风等
        icon = &icon_na;
    uiShowImageParamSendToQueue(166, 240, icon);
}
