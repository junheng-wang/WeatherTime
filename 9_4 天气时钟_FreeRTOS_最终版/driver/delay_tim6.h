#ifndef __DELAY_TIM6_H__
#define __DELAY_TIM6_H__

#include <stdint.h>


typedef void (*tim6_periodic_callback_t)(void);


void initTim6(void);
uint64_t getCurrentTim6Count(void);
uint64_t getPassedTimeUseUsUnit(void);
uint64_t getPassedTimeUseMsUnit(void);
void delayUsUseTim6(uint32_t us);
void delayMsUseTim6(uint32_t ms);
void registerTim6PeriodicCallback(tim6_periodic_callback_t callback);

#endif
