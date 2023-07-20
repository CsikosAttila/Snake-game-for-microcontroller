#ifndef UART_COMM_H
#define UART_COMM_H

// Configure the USART peripheral
void UART0_Init(void);

// USART interrupt handler
void UART0_RX_IRQHandler(void);

// Print to serial
void print(const char* buff);
void printLine(const char* buff);
void printNumber(int n);

#endif /*UART_COMM_H*/
