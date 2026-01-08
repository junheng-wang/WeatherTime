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
	UiActionType 
}


static void uiFunc(void* param)
{
	while(1)
	{
		
	}
	
}

void initUi(void)
{
	xTaskCreate(uiFunc, "ui", 2048, NULL, 8, NULL);
}
