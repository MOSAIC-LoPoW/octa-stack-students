#include "uart.h"

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
void USB_UART_Init(uint32_t aBaudRate)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  USB_UART.Instance = LPUART1;
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
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
void P1_UART_Init(uint32_t aBaudRate)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  P1_UART.Instance = UART5;
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
  /* USER CODE BEGIN UART5_Init 2 */
  HAL_NVIC_SetPriority(UART5_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(UART5_IRQn);

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void P2_UART_Init(uint32_t aBaudRate)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */
  /* USER CODE END USART2_Init 1 */
  P2_UART.Instance = USART2;
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

  /* USER CODE BEGIN USART2_Init 2 */
  HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
void P3_UART_Init(uint32_t aBaudRate)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  P3_UART.Instance = USART3;
  P3_UART.Init.BaudRate = aBaudRate;
  P3_UART.Init.WordLength = UART_WORDLENGTH_8B;
  P3_UART.Init.StopBits = UART_STOPBITS_1;
  P3_UART.Init.Parity = UART_PARITY_NONE;
  P3_UART.Init.Mode = UART_MODE_TX_RX;
  P3_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  P3_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  P3_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  P3_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&P3_UART) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  HAL_NVIC_SetPriority(USART3_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE END USART3_Init 2 */

}


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void BLE_UART_Init(uint32_t aBaudRate)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  BLE_UART.Instance = USART2;
  BLE_UART.Init.BaudRate = aBaudRate;
  BLE_UART.Init.WordLength = UART_WORDLENGTH_8B;
  BLE_UART.Init.StopBits = UART_STOPBITS_1;
  BLE_UART.Init.Parity = UART_PARITY_NONE;
  BLE_UART.Init.Mode = UART_MODE_TX_RX;
  BLE_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  BLE_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  BLE_UART.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  BLE_UART.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&BLE_UART) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USART2_Init 2 */
  HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE END USART2_Init 2 */

}

// WEAK UART RX CALLBACK
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  printf("uart interrupt callback not implemented in application\r\n");
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
  HAL_UART_Transmit(&USB_UART, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}