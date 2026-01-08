#include "board.h"

// ²âÊÔº¯Êý1
void set1_led(void)
{
	openLed(&led1);
	openLed(&led2);
	closeLed(&led3);
}
// ²âÊÔº¯Êý2
void set2_led(void)
{
	openLed(&led1);
	closeLed(&led2);
	openLed(&led3);
}
// ²âÊÔº¯Êý3
void set3_led(void)
{
	closeLed(&led1);
	openLed(&led2);
	openLed(&led3);
}

int main(void)
{
	initBoardLowLevel();
	
	initLed(&led1); closeLed(&led1);
	initLed(&led2); closeLed(&led2);
	initLed(&led3); closeLed(&led3);
	
	initKey(&key1);
	initKey(&key2);
	initKey(&key3);
	
	
	// initKey();
	
	key_press_callback_register(&key1, set1_led);
	key_press_callback_register(&key2, set2_led);
	key_press_callback_register(&key3, set3_led);
	
	while(1)
	{
		
	}
}
