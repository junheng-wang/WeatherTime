#include "board.h"
#include "show_page.h"
#include "system_operation_scheduling.h"


static void startSystem(void* param)
{
	initExternalDevices();
	
	// 展示默认页面
	showMainPage();
	
	creatSystemSchedulingLogic();
	
	vTaskDelete(NULL);
}


int main(void)
{
	initBoardOnLowLevel();
	
	xTaskCreate(startSystem, "Start the System", 1024, NULL, 9, NULL);
	
	vTaskStartScheduler();
	
	while(1)
	{
		;	// code should not run to here		
	}

}

void vAssertCalled(const char *file, int line)
{
    printf("Assert Called: %s(%d)\n", file, line);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName)
{
    printf("Stack Overflowed: %s\n", pcTaskName);
    configASSERT(0);
}

void vApplicationMallocFailedHook( void )
{
    printf("Malloc Failed\n");
    configASSERT(0);
}
