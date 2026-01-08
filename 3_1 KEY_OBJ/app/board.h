#ifndef __BOARD_H__
#define __BOARD_H__

#include "led.h"
#include "key.h"

extern led_desc_t led1;
extern led_desc_t led2;
extern led_desc_t led3;

extern key_t key1;
extern key_t key2;
extern key_t key3;

void initBoardLowLevel(void);

#endif
