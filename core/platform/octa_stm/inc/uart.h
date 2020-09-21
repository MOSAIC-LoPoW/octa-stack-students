#include "stm32l4xx_hal.h"

UART_HandleTypeDef USB_UART;
UART_HandleTypeDef NBIOT_UART;
UART_HandleTypeDef RS232_UART;
UART_HandleTypeDef P1_UART;
UART_HandleTypeDef P2_UART;

void USB_UART_Init(uint32_t aBaudRate);
void NBIOT_UART_Init(uint32_t aBaudRate);
void RS232_UART_Init(uint32_t aBaudRate);
void P1_UART_Init(uint32_t aBaudRate);
void P2_UART_Init(uint32_t aBaudRate);
void UART_SetApplicationCallback(void *appcallback, uint8_t connector);
void UART_SetShieldCallback(void *shieldcallback, uint8_t connector);
void UART_USB_SetRxCallback(void *rxcallback);
void UART_NBIOT_SetRxCallback(void *rxcallback);

int _write(int file, char *ptr, int len);
int __io_putchar(int ch);