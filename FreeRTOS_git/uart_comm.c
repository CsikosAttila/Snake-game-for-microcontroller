#include "uart_comm.h"

#include <stdio.h>
#include <stdint.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#include "snake/game_controller.h"

#define UART0_PORT      gpioPortE
#define UART0_TX_PIN    0
#define UART0_RX_PIN    1
#define UART0_BAUDRATE  115200
#define DATA_BUFF_SIZE  3

#define UART_PRIO 2

void UART0_RX_IRQHandler(void)
{
    BaseType_t switch_req;

    // Check if RX data is available
    if (UART0->STATUS & USART_STATUS_RXDATAV)
    {
        // Read the received data
        uint8_t rxData = USART_Rx(UART0);

        switch_req = sendKeyPressFromIsr(rxData);
    }

    USART_IntClear(UART0, USART_IF_RXDATAV);

    //switch_req = xTaskResumeFromISR(UART_handle);
    portYIELD_FROM_ISR(switch_req);
}

void UART0_Init(void)
{
      // Enable clock for UART0
      CMU_ClockEnable(cmuClock_UART0, true);

      // Configure UART0 TX pin
      GPIO_PinModeSet(UART0_PORT, UART0_TX_PIN, gpioModePushPull, 1);

      // Configure UART0 RX pin
      GPIO_PinModeSet(UART0_PORT, UART0_RX_PIN, gpioModeInput, 0);

      // Configure UART0
      USART_InitAsync_TypeDef uartInit = USART_INITASYNC_DEFAULT;
      uartInit.baudrate = UART0_BAUDRATE;
      USART_InitAsync(UART0, &uartInit);

      // Route to loc 1, rx tx en
      UART0->ROUTE |= USART_ROUTE_LOCATION_LOC1 | USART_ROUTE_RXPEN | USART_ROUTE_TXPEN;

      // Enable UART0 TX and RX interrupts
      USART_IntEnable(UART0, USART_IEN_RXDATAV);

      // Enable UART0 interrupt vectors in NVIC
      NVIC_ClearPendingIRQ(UART0_RX_IRQn);
      NVIC_EnableIRQ(UART0_RX_IRQn);

      // Redirecting UART
      GPIO_PinModeSet(gpioPortF, 7, gpioModePushPull, 1);

      // Enable UART0
      USART_Enable(UART0, usartEnable);
}

void printChar(char c)
{
    USART_Tx(UART0, c);
}

// Print to serial
void print(const char* buff)
{
    for (int i = 0; buff[i] != '\0'; i++)
        printChar(buff[i]);
}

void printLine(const char* buff)
{
    print(buff);
    print("\r\n");
}

void printNumber(int n)
{
    char str[10];
    sprintf(str, "%d", n);
    print(str);
}
