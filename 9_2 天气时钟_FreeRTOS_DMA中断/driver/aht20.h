#ifndef __AHT20_H__
#define __AHT20_H__


#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"


#define AHT20_ADDR	0x70


uint8_t initAht20(void);
uint8_t startMeasureOfAht20(uint8_t time_out_ms);
uint8_t readMeasureData(float* temperature, float* humidity);


#endif
