#include <stdint.h>
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#include "i2c_peripheral.h"

volatile uint8_t tx_data[] = {0xAD, 0x55};
volatile uint8_t rx_data[2] = {0x00, 0x00};

int main(void)
{
	volatile uint8_t a = 0;
	initI2c1();
	writeMulByteByI2c1(0x0011, tx_data, 2);
	for(uint32_t i=0; i<2000000; i++);
	readMulByteByI2c1(0x0011, rx_data, 2);
	
	if(rx_data[0]==0xAD && rx_data[1] == 0x55)
	{
		a = 1;
	}
	else
	{
		a = 0;
	}
	
	while(1)
	{
		
	}
}

//volatile uint8_t a = 0;

//initI2c1();
//writeOneByteByI2c1(0x0011, 0xAB);
//for(uint32_t i=0; i<2000000; i++);
//volatile uint8_t rx_data =  readOneByteByI2c1(0x0011);

//if(rx_data == 0xAB)
//{
//	a = 1;
//}
//else
//{
//	a = 0;
//}
//	
