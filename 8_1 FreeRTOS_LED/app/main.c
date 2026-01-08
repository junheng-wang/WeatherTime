#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"


typedef struct{
	led_desc_t* led;
	uint16_t led_delay_ms;
}TempLedType;

static void led_blink_task(void* param)
{
	TempLedType* p_t_led = (TempLedType*)param;
	
	while(1)
	{
		openLed(p_t_led->led);
		vTaskDelay(pdMS_TO_TICKS(p_t_led->led_delay_ms));	// —” ±500ms
		closeLed(p_t_led->led);
		vTaskDelay(pdMS_TO_TICKS(p_t_led->led_delay_ms));	// —” ±500ms
	}
}


int main(void)
{
    initBoardOnLowLevel();
	initExternalDevices();
	
	initLed(&led1);
	initLed(&led2);
	initLed(&led3);
	
	closeLed(&led1);
	closeLed(&led2);
	closeLed(&led3);
	
	
	static TempLedType t_led1 = {&led1, 250};
	static TempLedType t_led2 = {&led2, 500};
	static TempLedType t_led3 = {&led3, 1000};
	
	
	xTaskCreate(led_blink_task, "led1_blink", 512, &t_led1, 5, NULL);
	xTaskCreate(led_blink_task, "led2_blink", 512, &t_led2, 5, NULL);
	xTaskCreate(led_blink_task, "led3_blink", 512, &t_led3, 5, NULL);
	
	
	vTaskStartScheduler();
    
    return 0;
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
