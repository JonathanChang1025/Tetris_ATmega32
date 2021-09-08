#ifndef USART_H_
#define USART_H_

char* itoa(int value, char* result, int base);
void USART_init(int baud_prescaller);
unsigned char USART_receive(void);
void USART_send(unsigned char data);
void USART_putstring(char* StringPtr);

#endif