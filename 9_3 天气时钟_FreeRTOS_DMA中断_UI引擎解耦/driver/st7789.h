#ifndef __ST7789_H__
#define __ST7789_H__

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"
#include "font.h"
#include "image.h"


#define ST7789_WIDTH	240
#define ST7789_HEIGHT	320
#define mkcolor(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))


void initSt7798(void);
void resetSt7789(void);
void fillColorForSt7789(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void setBackGroundLed(uint8_t on_off);
void showString(uint16_t x, uint16_t y, char* string, uint16_t color, uint16_t bg_color, const FontType* font);
void showImage(uint16_t x, uint16_t y, const ImageType* image);

#endif
