#pragma anon_unions

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ui.h"
#include "st7789.h"


// 定义一个枚举变量：将刷屏、显示文字、显示图片三个动作用枚举值代替
typedef enum{
	UI_ACTION_FILL_COLOR,
	UI_ACTION_SHOW_STRING,
	UI_ACTION_SHOW_IMAGE
}UiActionType;

// 定义一个结构体变量：保存要执行的屏幕动作，以及该动作所用的参数
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


// 定义一个全局静态变量：ui队列，队列中存放的内容就是上面的结构体变量
static QueueHandle_t ui_queue;


/**
 *@brief UI的任务函数
 *@note1 当队列中没有数据的时候就阻塞等待，释放占用的CPU资源
 *@note2 当队列中有数据后，则会唤醒此任务（此任务不一定立即执行，要等FreeRTOS调度），然后执行响应的屏幕显示动作
 */
static void uiTaskFunc(void* param)
{
	// 初始化ST7789外部模块，保证屏幕能正常显示
	initSt7798();
	// 定义一个局部变量：用于接收队列中的数据
	UiMessageType ui_msg;
	
	// 超级循环，用于不断检测队列中是否有数据，注意必须使用超级循环，这是一个任务函数，不能退出，若要退出必须自杀
	while(1)
	{
		// 取队列的数据
		xQueueReceive(ui_queue, &ui_msg, portMAX_DELAY);
		// 根据第一个枚举值决定屏幕做什么动作
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

/**
 *@brief 初始化UI
 */
void initUi(void)
{
	// 创建一个队列
	ui_queue = xQueueCreate(16, sizeof(UiMessageType));
	configASSERT(ui_queue);
	
	// 创建一个任务
	xTaskCreate(uiTaskFunc, "ui show task", 1024, NULL, 8, NULL);
}

/**
 *@brief 向ui队列写入FillColor和FillColor的参数
 *@note1 调用此函数后，队列中就有一个成员了，该成员指定了屏幕要执行的动作是FillColor，执行该动作的参数也写入其中了
 *@note2 向队列写入数据后，屏幕不一定能立马做出响应的动作，必须等待FreeRTOS系统调用到读取队列数据的任务
 */
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

/**
 *@brief 向ui队列写入ShowString和ShowString的参数
 *@note1 调用此函数后，队列中就有一个成员了，该成员指定了屏幕要执行的动作是ShowString，执行该动作的参数也写入其中了
 *@note2 向队列写入数据后，屏幕不一定能立马做出响应的动作，必须等待FreeRTOS系统调用到读取队列数据的任务
 */
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

/**
 *@brief 向ui队列写入ShowImage和ShowImage的参数
 *@note1 调用此函数后，队列中就有一个成员了，该成员指定了屏幕要执行的动作是ShowImage，执行该动作的参数也写入其中了
 *@note2 向队列写入数据后，屏幕不一定能立马做出响应的动作，必须等待FreeRTOS系统调用到读取队列数据的任务
 */
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
