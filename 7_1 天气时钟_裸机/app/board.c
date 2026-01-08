#include <stdio.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "board.h"


/**
 *@brief 初始化所有的用到的外设的时钟
 */
void initPeripheralRccClock(void)
{
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	// ===================== RTC时钟部分 ========================
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
}

/**
 *@brief 初始化所有用到的NVIC配置
 */
void initPeripheralNvic(void)
{
	// 设置中断优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 *@brief 板级底层初始化
 *@note1 这里用于所有外设时钟RCC和NVIC的初始化了
 */
void initBoardOnLowLevel(void)
{
	initPeripheralRccClock();
	initPeripheralNvic();
}

/**
 *@brief 初始化ESP32及其WiFi和SNTP模块
 *@note1 因为Wifi和SNTP模块的初始化和普通外设、外部模块不同，有很大可能会失败，因此单独编写函数
 */
void initWirelessUseEsp32(void)
{
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
	
	if(setTimeZoneAndSntpServer() == 0)
	{
        printf("[SNTP] init failed\r\n");
        goto err;
    }
	printf("[SNTP] inited\r\n");
	return;
	
err:
	showWirelessErrorPage();
    while (1)
    {
        printf("ESP32 Error\r\n");
        delayMsUseSystick(1000);
    }
}

/**
 *@brief 等待WiFi连接，等待时间为10s，10s还是连接失败，则系统卡死
 */
void waitWifiConnect(void)
{
	// 等待Wifi连接
	if (connectWifiUseEsp32(WIFI_SSID, WIFI_PASSWD, NULL) == 0)
    {
        printf("[WIFI] connect failed\r\n");
		goto wrong;
    }
    printf("[WIFI] connecting\r\n");
	
	// 打印Wifi信息
	for (uint32_t t=0; t<10*1000; t+=100)
    {
        delayMsUseSystick(100);
        WifiInfoType wifi_info = { 0 };
        if (getWifiInfo(&wifi_info) && wifi_info.connected)
        {
            printf("[WIFI] Connected\n");
            printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                wifi_info.ssid, wifi_info.bssid, wifi_info.channel, wifi_info.rssi);
            return;
        }
    }
	
wrong:
    printf("[WIFI] Connection Timeout\n");
	// Wifi连接失败页面展示
    showWirelessErrorPage();
	// 系统卡死
    while (1)
    {
		
    }
}

/**
 *@brief 初始化系统，包括外部模块、各种初始显示
 */
void initExternalDevices(void)
{
	// 1. 初始化systick时钟
	delayOneMsUseInterruptModeInit();
	// 2. 初始化USART1，用于日志调试printf
	initUsart1();
	// 3. 初始化RTC
	initRtc();
	// 4. 初始化AHT20
	initAht20();
	// 5. 初始化LCD屏幕
	initSt7798();
	// 6. LCD屏幕显示欢迎页面
	showWelcomePage();
	// 7. 初始化Wifi和SNTP工作模式
	initWirelessUseEsp32();
	// 8. 显示Wifi连接页面
	showWaitWifiConnectPage();
	// 9. 连接Wifi
	waitWifiConnect();
	
	printf("[SYS] Build Date: %s %s\n", __DATE__, __TIME__);
}
