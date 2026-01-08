#ifndef __I2C_PERIPHERAL_H__
#define __I2C_PERIPHERAL_H__

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_conf.h"

#define BL24C512A_ADDR	0xA0

void initI2c1(void);
void writeOneByteByI2c1(uint16_t reg_addr, uint8_t data);
void writeMulByteByI2c1(uint16_t reg_addr, uint8_t data[], uint8_t len);
uint8_t readOneByteByI2c1(uint16_t reg_addr);
void readMulByteByI2c1(uint16_t reg_addr, uint8_t data[], uint8_t len);

#endif
