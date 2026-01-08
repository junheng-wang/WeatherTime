#ifndef __UI_H__
#define __UI_H__


#include "font.h"
#include "image.h"


void initUi(void);
void uiFillColorParamSendToQueue(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void uiShowStringParamSendToQueue(uint16_t x, uint16_t y, char* string, uint16_t color, uint16_t bg_color, const FontType* font);
void uiShowImageParamSendToQueue(uint16_t x, uint16_t y, const ImageType* image);


#endif
