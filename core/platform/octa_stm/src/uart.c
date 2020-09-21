#include "platform.h"

static void (*P1_Shield_UART_RX_Callback)(void) = 0;
static void (*P1_Application_UART_RX_Callback)(void) = 0;
static void (*P2_Shield_UART_RX_Callback)(void) = 0;
static void (*P2_Application_UART_RX_Callback)(void) = 0;
static void (*USB_UART_RX_Callback)(void) = 0;
static void (*NBIOT_UART_RX_Callback)(void) = 0;

void USB_UART_Init(uint32_t aBaudRate)
{
  USB_UART.Instance = USART2;
  USB_UART.Init.BaudRate = aBaudRate;
  USB_UART.Init.WordLength = UART_WORDLENGTH_8B;
  USB_UART.Init.StopBits = UART_STOPBITS_1;
  USB_UART.Init.Parity = UART_PARITY_NONE;
  USB_UART.Init.Mode = UART_MODE_TX_RX;
  USB_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  USB_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  USB_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&USB_UART) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void NBIOT_UART_Init(uint32_t aBaudRate)
{
  NBIOT_UART.Instance = LPUART1;
  NBIOT_UART.Init.BaudRate = aBaudRate;
  NBIOT_UART.Init.WordLength = UART_WORDLENGTH_8B;
  NBIOT_UART.Init.StopBits = UART_STOPBITS_1;
  NBIOT_UART.Init.Parity = UART_PARITY_NONE;
  NBIOT_UART.Init.Mode = UART_MODE_TX_RX;
  NBIOT_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  NBIOT_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  NBIOT_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&NBIOT_UART) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_NVIC_SetPriority(LPUART1_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);
}

void RS232_UART_Init(uint32_t aBaudRate)
{
  RS232_UART.Instance = UART4;
  RS232_UART.Init.BaudRate = aBaudRate;
  RS232_UART.Init.WordLength = UART_WORDLENGTH_8B;
  RS232_UART.Init.StopBits = UART_STOPBITS_1;
  RS232_UART.Init.Parity = UART_PARITY_NONE;
  RS232_UART.Init.Mode = UART_MODE_TX_RX;
  RS232_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  RS232_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  RS232_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  RS232_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&RS232_UART) != HAL_OK)
  {
    Error_Handler();
  }
}

void P1_UART_Init(uint32_t aBaudRate)
{
  P1_UART.Instance = USART3;
  P1_UART.Init.BaudRate = aBaudRate;
  P1_UART.Init.WordLength = UART_WORDLENGTH_8B;
  P1_UART.Init.StopBits = UART_STOPBITS_1;
  P1_UART.Init.Parity = UART_PARITY_NONE;
  P1_UART.Init.Mode = UART_MODE_TX_RX;
  P1_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  P1_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  P1_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  P1_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&P1_UART) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_NVIC_SetPriority(USART3_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void P2_UART_Init(uint32_t aBaudRate)
{
  P2_UART.Instance = USART1;
  P2_UART.Init.BaudRate = aBaudRate;
  P2_UART.Init.WordLength = UART_WORDLENGTH_8B;
  P2_UART.Init.StopBits = UART_STOPBITS_1;
  P2_UART.Init.Parity = UART_PARITY_NONE;
  P2_UART.Init.Mode = UART_MODE_TX_RX;
  P2_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  P2_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  P2_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  P2_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&P2_UART) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

// UART RX CALLBACK
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &P1_UART)
  {
    if(P1_Shield_UART_RX_Callback)
      P1_Shield_UART_RX_Callback();
    if(P1_Application_UART_RX_Callback)
      P1_Application_UART_RX_Callback();
  }
  else if (huart == &P2_UART)
  {
    if(P2_Shield_UART_RX_Callback)
      P2_Shield_UART_RX_Callback();
    if(P2_Application_UART_RX_Callback)
      P2_Application_UART_RX_Callback();
  }
  else if (huart == &USB_UART)
  {
    if(USB_UART_RX_Callback)
      USB_UART_RX_Callback();
  }
  else if (huart == &NBIOT_UART)
  {
    if(NBIOT_UART_RX_Callback)
      NBIOT_UART_RX_Callback();
  }
  else 
  {
    printERR("UART Handle unknown\r\n");
    Error_Handler();
  }
}

void UART_SetApplicationCallback(void (*appcallback), uint8_t connector)
{
  switch(connector)
  {
    case 1:
      P1_Application_UART_RX_Callback = appcallback;
      break;
    case 2:
      P2_Application_UART_RX_Callback = appcallback;
      break;
    default:
      break;
  }
}

void UART_SetShieldCallback(void (*shieldcallback), uint8_t connector)
{
  switch(connector)
  {
    case 1:
      P1_Shield_UART_RX_Callback = shieldcallback;
      break;
    case 2:
      P2_Shield_UART_RX_Callback = shieldcallback;
      break;
    default:
      break;
  }
}

void UART_USB_SetRxCallback(void (*rxcallback))
{
  USB_UART_RX_Callback = rxcallback;
}

void UART_NBIOT_SetRxCallback(void (*rxcallback))
{
  NBIOT_UART_RX_Callback = rxcallback;
}

/*******************************************
PRINTF TO SERIAL REDIRECT FUNCTION PROTOTYPE 
********************************************/
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/* required for printf to work */
int _write(int file, char *ptr, int len)
{
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&USB_UART, (uint8_t *)&ch, 1, 500);
  return ch;
}