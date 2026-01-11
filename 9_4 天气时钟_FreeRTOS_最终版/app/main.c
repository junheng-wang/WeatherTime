#include "board.h"
#include "show_page.h"
#include "system_operation_scheduling.h"
#include "work_queue.h"


static void startSystem(void* param)
{
	// 初始化外设和外部模块
	initExternalDevices();
	
	// 初始化在屏幕上显示默认页面
	showMainPage();
	
	// 创建系统运行调度逻辑
	creatSystemSchedulingLogic();
	
	// 此任务配置好外设等参数、创建好系统接下来要执行的任务后，这个初始化任务就可以结束了
	vTaskDelete(NULL);
}


int main(void)
{
	// 底层板级初始化
	initBoardOnLowLevel();
	// 初始化系统主要的工作任务
	initSystemWork();
	// 初始化系统
	xTaskCreate(startSystem, "Start the System", 1024, NULL, 9, NULL);
	// 启动调度器
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
