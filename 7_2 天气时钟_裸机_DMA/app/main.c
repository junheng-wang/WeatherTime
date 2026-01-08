#include "board.h"
#include "show_page.h"
#include "system_operation_scheduling.h"


int main(void)
{
	initBoardOnLowLevel();
	initExternalDevices();
	
	// 展示默认页面
	showMainPage();
	// 注册Systick回调函数
	forSystickRegisterCallback();
	
	
	while(1)
	{
		// 开始系统调度
		systemSchedulingLogic();
		// printf("Juneheng\r\n");
		// delayMsUseSystick(1500);
	}

}
