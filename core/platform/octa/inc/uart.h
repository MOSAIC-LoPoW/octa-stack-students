#include "stm32l4xx_hal.h"

UART_HandleTypeDef USB_UART;
UART_HandleTypeDef BLE_UART;
UART_HandleTypeDef FTDI_UART;
UART_HandleTypeDef P1_UART;
UART_HandleTypeDef P2_UART;
UART_HandleTypeDef P3_UART;

void USB_UART_Init(uint32_t aBaudRate);
void BLE_UART_Init(uint32_t aBaudRate);
void FTDI_UART_Init(uint32_t aBaudRate);
void P1_UART_Init(uint32_t aBaudRate);
void P2_UART_Init(uint32_t aBaudRate);
void P3_UART_Init(uint32_t aBaudRate);
void UART_SetApplicationCallback(void (*appcallback), uint8_t connector);
void UART_SetShieldCallback(void (*shieldcallback), uint8_t connector);
void UART_BLE_SetRxCallback(void (*rxcallback));
void UART_USB_SetRxCallback(void (*rxcallback));
void UART_FTDI_SetRxCallback(void (*rxcallback));

int _write(int file, char *ptr, int len);
int __io_putchar(int ch);