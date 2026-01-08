#ifndef __USART1_H__
#define __USART1_H__

typedef void(*console_received_func_t)(uint8_t data);

void initUsart1(void);
void sendDataByUsart1(const char* str);
uint8_t receiveDataByUsart1(void);
void registerReceiveFunForUsart1(console_received_func_t func);

#endif
