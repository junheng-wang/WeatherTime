#include <stdio.h>
#include <string.h>
#include "st7789.h"
#include "systick_delay.h"


#define ST7789_WIDTH	240
#define ST7789_HEIGHT	320

/* =============================
 * LCD的部分引脚连接说明
 *	// LCD_SLK		PB13	SPI2_SLK
 *	// LCD_MOSI		PC3		SPI2_MOSI
 *	// LCD_MISO		PC2		SPI2_MISO	
 *	// LCD_CS		PE2		SPI2_CS
 *	// LCD_REST		PE3		
 *	// LCD_DC		PE4		
 *	// LED			PE5
 * ============================= */
#define CS_PORT		GPIOE
#define CS_PIN		GPIO_Pin_2
#define RESET_PORT	GPIOE
#define RESET_PIN	GPIO_Pin_3
#define DC_PORT		GPIOE
#define DC_PIN		GPIO_Pin_4
#define LED_PORT	GPIOE
#define LED_PIN		GPIO_Pin_5


void initSpi2(void);
void writeDataToSt7789(uint8_t reg, uint8_t data[], uint16_t len);
uint8_t isExceedsScreenBoundary(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void setRangeForSt7789(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void sendFontDataToSt7789(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* model, uint16_t color, uint16_t bg_color);
void showOneAscii(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color, const FontType* font);
const uint8_t* searchChinese(char* ch, const FontType* font);
void showOneChinese(uint16_t x, uint16_t y, char* ch, uint16_t color, uint16_t bg_color, const FontType* font);


/**
 *@brief 初始化st7789
 */
void initSt7798(void)
{
	// 1. 初始化通信外设SPI2和相关引脚
	initSpi2();
	
	// 2. 复位ST7789
	resetSt7789();
	
	// 3. 下面进行一系列初始化配置（屏幕厂家给的示例代码）
	writeDataToSt7789(0x11, NULL, 0);
	delay_ms(5);
	writeDataToSt7789(0x36, (uint8_t[]){0x00}, 1);
	writeDataToSt7789(0x3A, (uint8_t[]){0x55}, 1);
	writeDataToSt7789(0xB2, (uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
	writeDataToSt7789(0xB7, (uint8_t[]){0x46}, 1);
	writeDataToSt7789(0xBB, (uint8_t[]){0x1B}, 1);
	writeDataToSt7789(0xC0, (uint8_t[]){0x2C}, 1);
	writeDataToSt7789(0xC2, (uint8_t[]){0x01}, 1);
	writeDataToSt7789(0xC3, (uint8_t[]){0x0F}, 1);
	writeDataToSt7789(0xC4, (uint8_t[]){0x20}, 1);
	writeDataToSt7789(0xC6, (uint8_t[]){0x0F}, 1);
	writeDataToSt7789(0xD0, (uint8_t[]){0xA4, 0xA1}, 2);
	writeDataToSt7789(0xD6, (uint8_t[]){0xA1}, 1);
	writeDataToSt7789(0xE0, (uint8_t[]){0xF0,0x00,0x06,0x04,0x05,0x05,0x31,0x44,0x48,0x36,0x12,0x12,0x2B,0x34}, 14);
	writeDataToSt7789(0xE1, (uint8_t[]){0xF0,0x0B,0x0F,0x0F,0x0D,0x26,0x31,0x43,0x47,0x38,0x14,0x14,0x2C,0x32}, 14);
	writeDataToSt7789(0x21, NULL, 0);
	writeDataToSt7789(0x29, NULL, 0);
	
}


/**
 *@brief 初始化SPI2，用于与ST7789通信
 */
void initSpi2(void)
{
	// 1. 开启硬件SPI1时钟对应的GPIO时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	// 2. 设置GPIO端口的复用模式
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);
	
	// 3. 设置GPIO的工作模式
	GPIO_InitTypeDef GPIO_InitStruct;
	// 3.1 SLK引脚PB13
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// 3.2 MOSI引脚PC3和MISO引脚PC2
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	// 3.3 片选引脚PE2、复位引脚PE3、DC引脚PE4、LED背光引脚PE5
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	// 3.4 设置默认电平
	GPIO_SetBits(GPIOE, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4);
	GPIO_ResetBits(GPIOE, GPIO_Pin_5);
	
	// 4. 设置硬件SPI的工作模式
	SPI_InitTypeDef SPI_InitStruct;
	SPI_StructInit(&SPI_InitStruct);
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI2, &SPI_InitStruct);
	
	// 5. 使能SPI
	SPI_Cmd(SPI2, ENABLE);
}

/**
 *@brief 向st7789指定寄存器地址写入多个字节的数据
 *@param reg 命令或者寄存器地址
 *@param data 数据
 *@param len 写入数据的字节数
 */
void writeDataToSt7789(uint8_t reg, uint8_t data[], uint16_t len)
{
	// 拉低片选信号
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_RESET);
	// 拉低DC引脚，表明发送的是命令
	GPIO_WriteBit(DC_PORT, DC_PIN, Bit_RESET);
	
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(SPI2, reg);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET);
	
	// 拉高DC引脚，表明发送的是数据
	GPIO_WriteBit(DC_PORT, DC_PIN, Bit_SET);
	// 发送数据
	for(uint16_t i=0; i<len; i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
		SPI_I2S_SendData(SPI2, data[i]);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	}
	// 确认传输完全结束
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET);
	
	// 拉高片选信号，结束通信
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);
}


/**
 *@brief 复位st7789
 */
void resetSt7789(void)
{
	// reset引脚低电平复位
	GPIO_WriteBit(RESET_PORT, RESET_PIN, Bit_RESET);
	// 保持10us以上
	delay_us(15);
	// reset引脚置高，停止复位
	GPIO_WriteBit(RESET_PORT, RESET_PIN, Bit_SET);
	// 延迟120ms
	delay_ms(120);
}

/**
 *@brief 判断填充屏幕的区域范围是否超出屏幕尺寸
 *@param x1 左上角横坐标
 *@param y1 左上角纵坐标
 *@param x2 右上角横坐标
 *@param y2 右上角纵坐标
 *@return 1-超出屏幕边界，0-没有超出屏幕边界
 */
uint8_t isExceedsScreenBoundary(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if(x1 >= ST7789_WIDTH || x2 >= ST7789_WIDTH || y1 >= ST7789_HEIGHT || y2 >= ST7789_HEIGHT)
		return 1;
	else
		return 0;
}

/**
 *@brief 设置填充屏幕的区域x轴和y轴范围
 *@param x1 左上角横坐标
 *@param y1 左上角纵坐标
 *@param x2 右上角横坐标
 *@param y2 右上角纵坐标
 */
void setRangeForSt7789(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	// 设置x轴范围
	writeDataToSt7789(0x2A, (uint8_t[]){(x1 >> 8) & 0xFF, x1 & 0xFF, (x2 >> 8) & 0xFF, x2 & 0xFF}, 4);
	// 设置y轴范围
	writeDataToSt7789(0x2B, (uint8_t[]){(y1 >> 8) & 0xFF, y1 & 0xFF, (y2 >> 8) & 0xFF, y2 & 0xFF}, 4);
	// 发送写入命令
	writeDataToSt7789(0x2C, NULL, 0);
}

/**
 *@brief 给屏幕指定区域填充单一颜色（up参考的厂家给的示例代码）
 *@param x1 区域左上角横坐标
 *@param y1 区域左上角纵坐标
 *@param x2 区域右上角横坐标
 *@param y2 区域右上角纵坐标
 *@param color 区域填充颜色
 */
void fillColorForSt7789(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	// 计算填充区域像素个数
	uint32_t pixels_num = (x2 - x1 + 1) * (y2 - y1 + 1);
	// 提前把16位颜色值拆为两个8位数字
	uint8_t color_byte[2] = {(uint8_t)(color>>8), (uint8_t)color};
	
	// 判断区域是否超出屏幕边界
	if(isExceedsScreenBoundary(x1, y1, x2, y2) == 1)
		return;
	
	// 设置填充屏幕的区域x轴和y轴范围
	setRangeForSt7789(x1, y1, x2, y2);
	
	// 填充颜色
	// 使能片选信号
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_RESET);
	// 拉高DC线，表示发送的是数据
	GPIO_WriteBit(DC_PORT, DC_PIN, Bit_SET);
	// 向指定的寄存器内写入数据
	for(uint32_t i=0; i<pixels_num; i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
		SPI_I2S_SendData(SPI2, color_byte[0]);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
		SPI_I2S_SendData(SPI2, color_byte[1]);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		
	}
	// 确认传输完全结束
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET);
	
	// 拉高片选信号，结束通信
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);
}

/**
 *@brief 设置屏幕背光的亮灭
 *@param on_off 是否开启背光
 */
void setBackGroundLed(uint8_t on_off)
{
	GPIO_WriteBit(LED_PORT, LED_PIN, on_off>0 ? Bit_SET : Bit_RESET);
}

/**
 *@brief 将字符/汉字的字模数据通过SPI传输给ST7789
 *@param x 左上角横坐标
 *@param y 左上角纵坐标
 *@param width 字模宽度
 *@param height 字模高度
 *@param 对应字符/汉字的字模数据
 *@param color 字符颜色
 *@param bg_color 背景颜色
 */
void sendFontDataToSt7789(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* model, uint16_t color, uint16_t bg_color)
{
	// 局部变量：表示显示在屏幕上的字体像素颜色
	uint8_t color_data[] = {(color>>8)&0xFF, color&0xFF};
	// 局部变量：表示没有字体的方框内的其他像素颜色
	uint8_t bg_color_data[] = {(bg_color>>8)&0xFF, bg_color&0xFF};
	// 局部变量：表示字模的一行所用的字节数
	uint8_t per_row_byte = (width + 7) / 8;
	
	// 判断区域是否超出屏幕边界
	if(isExceedsScreenBoundary(x, y, x+width-1, y+height-1) == 1)
		return;
	
	// 设置填充屏幕的区域x轴和y轴范围
	setRangeForSt7789(x, y, x+width-1, y+height-1);
	
	// 填充颜色
	// 使能片选信号
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_RESET);
	// 拉高DC线，表示发送的是数据
	GPIO_WriteBit(DC_PORT, DC_PIN, Bit_SET);
	// 向指定的寄存器内写入数据
	for(uint16_t row=0; row<height; row++)
	{
		const uint8_t* model_addr = model + per_row_byte*row;
		for(uint16_t col=0; col<width; col++)
		{
			uint8_t font_mark = model_addr[col/8] & (1 << (7 - col%8));
			if(font_mark)
			{
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
				SPI_SendData(SPI2, color_data[0]);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
				SPI_SendData(SPI2, color_data[1]);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
			}
			else
			{
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
				SPI_SendData(SPI2, bg_color_data[0]);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
				SPI_SendData(SPI2, bg_color_data[1]);
				while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
			}
		}
	}
	// 确认传输完全结束
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET);
	
	// 拉高片选信号，结束通信
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);
}

/**
 *@brief 屏幕显示单个字符
 *@param x 左上角横坐标
 *@param y 左上角纵坐标
 *@param ch 要显示的字符
 *@param color 字符颜色
 *@param bg_color 背景颜色
 *@param font 要显示字符的字模
 */
void showOneAscii(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg_color, const FontType* font)
{
	// 局部变量：表示字模的一行所用的字节数
	uint8_t per_row_byte = (font->width + 7) / 8;
	// 局部变量：当前字符在字模数组中的起始位置
	const uint8_t* model_base_addr = font->ascii_model + (ch-' ') * font->height * per_row_byte;
	// 通过SPI传输数据给ST7789，在屏幕对应位置上显示英文字符
	sendFontDataToSt7789(x, y, font->width, font->height, model_base_addr, color, bg_color);
}

/**
 *@brief 从字模中取出汉字对应的字模
 *@note1 使用遍历搜索的方式
 */
const uint8_t* searchChinese(char* ch, const FontType* font)
{
	const ChineseFontType* p_start = font->chinese_model;
	// 遍历搜索
	while(p_start->name != NULL)
	{
		if(strcmp(ch, p_start->name) == 0)
			return p_start->model;
		p_start++;
	}
	// 若没有汉字对应的字模，则返回空
	return NULL;
}
/**
 *@brief 屏幕显示单个汉字
 *@param x 左上角横坐标
 *@param y 左上角纵坐标
 *@param ch 单个汉字
 *@param color 字符颜色
 *@param bg_color 背景颜色
 *@param font_size 要显示字符的字模
 */
void showOneChinese(uint16_t x, uint16_t y, char* ch, uint16_t color, uint16_t bg_color, const FontType* font)
{
	// 局部变量：当前字符在字模数组中的起始位置
	const uint8_t* model_base_addr = searchChinese(ch, font);
	if(model_base_addr == NULL)
		return;
	
	// 通过SPI传输数据给ST7789，在屏幕对应位置上显示中文汉字
	sendFontDataToSt7789(x, y, font->height, font->height, model_base_addr, color, bg_color);
}

/**
 *@brief 屏幕显示中英文字符串
 *@note1 这里不考虑utf8格式的汉字
 *@param x 左上角横坐标
 *@param y 左上角纵坐标
 *@param str 要显示的字符串
 *@param color 字符颜色
 *@param bg_color 背景颜色
 *@param font 要显示字符的字模
 */
void showString(uint16_t x, uint16_t y, char* string, uint16_t color, uint16_t bg_color, const FontType* font)
{
	uint8_t len;
	
	// 循环打印字符串
    while (*string)
    {
		// 1. 判断传入的字符串是 GB2312格式的汉字 还是 英文字符（len=2为汉字，len=1为英文）
		len = ((unsigned char)*string >= 0xA1 && (unsigned char)*string <= 0xF7) ? 2 : 1;
		
		if(len == 1)
		{
			showOneAscii(x, y, *string, color, bg_color, font);
			x += font->width;
			if(x >= ST7789_WIDTH-1-font->width)
			{
				x = 0;
				y += font->height;
			}
		}
		else
		{
			char chs[5];
            strncpy(chs, string, len);	// 复制指定长度字符串的函数，不够长度的复制后会用 \0 填充剩余空间
			showOneChinese(x, y, chs, color, bg_color, font);
			x += font->height;
			if(x >= ST7789_WIDTH-1-font->height)
			{
				x = 0;
				y += font->height;
			}
		}
        string += len;
    }
}

/**
 *@brief 屏幕显示图像
 *@param x 左上角横坐标
 *@param y 左上角纵坐标
 *@param image 图像数据
 */
void showImage(uint16_t x, uint16_t y, const ImageType* image)
{
	// 局部变量：图像的像素个数
	uint32_t pixel_num = image->width * image->height;
	
	// 判断区域是否超出屏幕边界
	if(isExceedsScreenBoundary(x, y, x+image->width-1, y+image->height-1) == 1)
		return;
	
	// 设置填充屏幕的区域x轴和y轴范围
	setRangeForSt7789(x, y, x+image->width-1, y+image->height-1);
	
	// 填充颜色
	// 使能片选信号
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_RESET);
	// 拉高DC线，表示发送的是数据
	GPIO_WriteBit(DC_PORT, DC_PIN, Bit_SET);
	// 向指定的寄存器内写入数据
	for(uint32_t i=0; i<pixel_num*2; i+=2)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
		SPI_SendData(SPI2, image->data[i+1]);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);
		SPI_SendData(SPI2, image->data[i]);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	}
	// 确认传输完全结束
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET);
	
	// 拉高片选信号，结束通信
	GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);
}

