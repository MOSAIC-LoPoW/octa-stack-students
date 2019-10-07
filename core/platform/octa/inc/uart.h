#include "stm32l4xx_hal.h"

UART_HandleTypeDef USB_UART;
UART_HandleTypeDef BLE_UART;
UART_HandleTypeDef P1_UART;
UART_HandleTypeDef P2_UART;
UART_HandleTypeDef P3_UART;

void USB_UART_Init(uint32_t aBaudRate);
void BLE_UART_Init(uint32_t aBaudRate);
void P1_UART_Init(uint32_t aBaudRate);
void P2_UART_Init(uint32_t aBaudRate);
void P3_UART_Init(uint32_t aBaudRate);