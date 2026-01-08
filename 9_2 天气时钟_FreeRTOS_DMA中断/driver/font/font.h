#ifndef __FONT_H__
#define __FONT_H__


#include <stdint.h>
#include <stdio.h>


typedef struct{
	char* name;
	const uint8_t* model;
}ChineseFontType;

typedef struct
{
    uint8_t height;
	uint8_t width;
	const char *ascii_map;
    const uint8_t* ascii_model;
	const ChineseFontType* chinese_model;
}FontType;

extern const FontType font16_maple;
extern const FontType font20_maple_bold;
extern const FontType font24_maple_bold;
extern const FontType font24_maple_semibold;
extern const FontType font32_maple_bold;
extern const FontType font54_maple_bold;
extern const FontType font54_maple_semibold;
extern const FontType font64_maple_extrabold;
extern const FontType font76_maple_extrabold;

#endif
