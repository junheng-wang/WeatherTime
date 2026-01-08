#ifndef __BOARD_H__
#define __BOARD_H__


#include "show_page.h"

#include "systick_delay.h"
#include "usart1.h"
#include "rtc.h"
#include "aht20.h"
#include "st7789.h"
#include "esp_at.h"


void initBoardOnLowLevel(void);
void initExternalDevices(void);


#endif
