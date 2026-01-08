#ifndef __FONT_H__
#define __FONT_H__


typedef struct{
	char* name;
	const uint8_t* model;
}ChineseFontType;


typedef struct{
	uint8_t height;
	uint8_t width;
	const uint8_t* ascii_model;
	const ChineseFontType* chinese_model;
}FontType;

extern const FontType font32;

#endif
