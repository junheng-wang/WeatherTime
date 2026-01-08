#ifndef __SYSTIC_DELAY_H__
#define __SYSTIC_DELAY_H__


#include <stdint.h>


typedef void (*systick_periodic_callback_t)(void);


void delayOneMsUseInterruptModeInit(void);
uint64_t getCurrentTickCount(void);
uint64_t getPassedTimeUseUsUnit(void);
uint64_t getPassedTimeUseMsUnit(void);
void delayUsUseSystick(uint32_t us);
void delayMsUseSystick(uint32_t ms);
void registerSystickPeriodicCallback(systick_periodic_callback_t callback);

//void delay_us(uint32_t us);
//void delay_ms(uint32_t ms);

#endif
