#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "work_queue.h"


/* 把所有想要后台执行的任务，全部放到work_queue中，让它们排队执行 */

// 定义一个结构体类型：保存一个函数句柄和函数相应的参数
// 这个函数句柄是系统需要执行的功能，比如：更新屏幕上的WiFi信息、更新屏幕上的时间信息、更新屏幕上的天气信息等
typedef struct{
	WorkType work;
	void* param;
}WorkMsgType;
// 定义一个队列，保存上面的结构体变量
static QueueHandle_t work_msg_queue;


/**
 *@brief 系统工作的任务函数
 *@note1 当队列中没有数据的时候就阻塞等待，释放占用的CPU资源
 *@note2 当队列中有数据后，则会唤醒此任务（此任务不一定立即执行，要等FreeRTOS调度），然后执行响应的屏幕显示动作
 */
static void systemWorkTaskFunc(void* param)
{
	// 定义一个局部变量：用于接收队列中的数据
	WorkMsgType work_msg;
	// 超级循环，用于不断检测队列中是否有数据，注意必须使用超级循环，这是一个任务函数，不能退出，若要退出必须自杀
	while(1)
	{
		// 阻塞等待从队列中取数据
		xQueueReceive(work_msg_queue, &work_msg, portMAX_DELAY);
		// 取到数据后就执行任务
		work_msg.work(work_msg.param);
	}
}

/**
 *@brief 初始化系统工作函数
 */
void initSystemWork(void)
{
	// 创建队列
	work_msg_queue = xQueueCreate(16, sizeof(WorkMsgType));
	configASSERT(work_msg_queue);
	
	// 创建任务
	xTaskCreate(systemWorkTaskFunc, "system work task", 1024, NULL, 5, NULL);
}

/**
 *@brief 将系统要执行的任务以及相应的参数写入工作队列
 *@note1 调用此函数后，队列中就有一个成员了，该成员指定了系统将要执行的动作，比如：更新屏幕上的WiFi信息、更新屏幕上的时间信息、更新屏幕上的天气信息等
 *@note2 向队列写入数据后，屏幕不一定能立马做出响应的动作，必须等待FreeRTOS系统调用到读取队列数据的任务
*/
void sendDataToWorkQueue(WorkType work, void* param)
{
	configASSERT(work_msg_queue);
	// 接收参数
	WorkMsgType work_msg = {work, param};
	// 将参数添加到队列
	xQueueSend(work_msg_queue, &work_msg, portMAX_DELAY);
}
