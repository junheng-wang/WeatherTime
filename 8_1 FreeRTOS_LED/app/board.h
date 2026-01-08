#ifndef __BOARD_H__
#define __BOARD_H__


#include "usart1.h"
#include "led.h"


extern led_desc_t led1;
extern led_desc_t led2;
extern led_desc_t led3;


void initBoardOnLowLevel(void);
void initExternalDevices(void);


#endif
