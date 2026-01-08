#ifndef __USART1_H__
#define __USART1_H__

void initUsart1(void);
void sendDataByUsart1(const char* str);
uint8_t receiveDataByUsart1(void);

#endif
