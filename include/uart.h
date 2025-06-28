#ifndef UART_H
#define UART_H

int printf(const char *format, ...);
int puts(const char *s);
void MX_USART1_UART_Init(void);

#endif // UART_H