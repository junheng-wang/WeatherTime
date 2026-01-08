#pragma anon_unions

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ui.h"
#include "st7789.h"



typedef enum{
	UI_ACTION_FILL_COLOR,
	UI_ACTION_SHOW_STRING,
	UI_ACTION_SHOW_IMAGE
}UiActionType;

typedef struct{
	UiActionType ui_action;
	union
	{
		struct
		{
			uint16_t x1; 
			uint16_t y1;
			uint16_t x2;
			uint16_t y2;
			uint16_t color;
		}fill_color;
		struct
		{
			uint16_t x;
			uint16_t y;
			char* str;
			uint16_t color;
			uint16_t bg_color;
			const FontType* font;
		}show_string;
		struct
		{
			uint16_t x; 
			uint16_t y;
			const ImageType* image;
		}show_image;
	};
		
}UiMessageType;


static QueueHandle_t ui_queue;


static void uiFunc(void* param)
{
	UiMessageType ui_msg;
	
	initSt7798();
	
	while(1)
	{
		// 取队列的数据
		xQueueReceive(ui_queue, &ui_msg, portMAX_DELAY);
		
		switch(ui_msg.ui_action)
		{
			case UI_ACTION_FILL_COLOR:
				fillColorForSt7789(ui_msg.fill_color.x1, ui_msg.fill_color.y1, ui_msg.fill_color.x2, ui_msg.fill_color.y2, ui_msg.fill_color.color);
				break;
			case UI_ACTION_SHOW_STRING:
				showString(ui_msg.show_string.x, ui_msg.show_string.y, ui_msg.show_string.str, ui_msg.show_string.color, ui_msg.show_string.bg_color, ui_msg.show_string.font);
				vPortFree((void*)ui_msg.show_string.str);
				break;
			case UI_ACTION_SHOW_IMAGE:
				showImage(ui_msg.show_image.x, ui_msg.show_image.y, ui_msg.show_image.image);
				break;
			default:
				printf("Unkown UI action: %d\n", ui_msg.ui_action);
				break;
		}
	}
	
}

void initUi(void)
{
	// 创建一个队列
	ui_queue = xQueueCreate(16, sizeof(UiMessageType));
	configASSERT(ui_queue);
	
	xTaskCreate(uiFunc, "ui", 2048, NULL, 8, NULL);
}

void uiFillColorParamSendToQueue(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	UiMessageType ui_msg;
	ui_msg.ui_action = UI_ACTION_FILL_COLOR;
	ui_msg.fill_color.x1 = x1;
	ui_msg.fill_color.y1 = y1;
	ui_msg.fill_color.x2 = x2;
	ui_msg.fill_color.y2 = y2;
	ui_msg.fill_color.color = color;
	
	// 向队列写入数据，不阻塞，写不进去直接返回
	xQueueSend(ui_queue, &ui_msg, 0);
}

void uiShowStringParamSendToQueue(uint16_t x, uint16_t y, char* string, uint16_t color, uint16_t bg_color, const FontType* font)
{
	char *pstr = pvPortMalloc(strlen(string) + 1);
    if (pstr == NULL)
    {
        printf("ui write string malloc failed: %s", string);
        return;
    }
    strcpy(pstr, string);
	
	UiMessageType ui_msg;
	ui_msg.ui_action = UI_ACTION_SHOW_STRING;
	ui_msg.show_string.x = x;
	ui_msg.show_string.y = y;
	ui_msg.show_string.str = pstr;
	ui_msg.show_string.color = color;
	ui_msg.show_string.bg_color = bg_color;
	ui_msg.show_string.font = font;
	
	// 向队列写入数据，不阻塞，写不进去直接返回
	xQueueSend(ui_queue, &ui_msg, 0);
}

void uiShowImageParamSendToQueue(uint16_t x, uint16_t y, const ImageType* image)
{
	UiMessageType ui_msg;
	ui_msg.ui_action = UI_ACTION_SHOW_IMAGE;
	ui_msg.show_image.x = x;
	ui_msg.show_image.y = y;
	ui_msg.show_image.image = image;
	
	// 向队列写入数据，不阻塞，写不进去直接返回
	xQueueSend(ui_queue, &ui_msg, 0);
}
