#include "gpio.h"

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void OCTA_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, P2_DIO1_Pin|P2_DIO6_Pin|P2_DIO2_Pin|P2_DIO4_Pin 
                          |P2_DIO3_Pin|P3_DIO1_Pin|P3_DIO4_Pin|P3_DIO5_Pin 
                          |P3_DIO2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, P1_DIO3_Pin|P1_DIO5_Pin|P2_DIO5_Pin|P3_DIO3_Pin , GPIO_PIN_SET);

  #if ENABLE_STEPUP
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_SET);
  #else
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_RESET);
  #endif
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, P1_DIO6_Pin|P1_DIO2_Pin|P1_DIO1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_Reset_GPIO_Port, BLE_Reset_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(P1_DIO4_GPIO_Port, P1_DIO4_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OCTA_GLED_Pin|P3_DIO6_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OCTA_RLED_Pin|OCTA_BLED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : P2_DIO1_Pin P2_DIO6_Pin P2_DIO2_Pin P2_DIO4_Pin 
                           P2_DIO3_Pin P3_DIO1_Pin P3_DIO4_Pin P3_DIO5_Pin 
                           P3_DIO2_Pin */
  GPIO_InitStruct.Pin = P2_DIO1_Pin|P2_DIO6_Pin|P2_DIO2_Pin|P2_DIO4_Pin 
                          |P2_DIO3_Pin|P3_DIO1_Pin|P3_DIO4_Pin|P3_DIO5_Pin 
                          |P3_DIO2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : P1_DIO3_Pin P1_DIO5_Pin P2_DIO5_Pin OCTA_STEPUP_Pin 
                           P3_DIO3_Pin */
  GPIO_InitStruct.Pin = P1_DIO3_Pin|P1_DIO5_Pin|P2_DIO5_Pin|OCTA_STEPUP_Pin 
                          |P3_DIO3_Pin|OCTA_LIGHTSENSORLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : P1_DIO6_Pin P1_DIO2_Pin P1_DIO1_Pin */
  GPIO_InitStruct.Pin = P1_DIO6_Pin|P1_DIO2_Pin|P1_DIO1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BLE_Reset_Pin P1_DIO4_Pin */
  GPIO_InitStruct.Pin = BLE_Reset_Pin|P1_DIO4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OCTA_GLED_Pin P3_DIO6_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = OCTA_GLED_Pin|P3_DIO6_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : OCTA_BTN1_Pin OCTA_BTN2_Pin */
  GPIO_InitStruct.Pin = OCTA_BTN1_Pin|OCTA_BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : OCTA_RLED_Pin OCTA_BLED_Pin */
  GPIO_InitStruct.Pin = OCTA_RLED_Pin|OCTA_BLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PF13 (OCTA-FLASH CS)*/
  GPIO_InitStruct.Pin = OCTA_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OCTA_FLASH_CS_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PE11 (OCTA-FLASH WP) PE0 (OCTA-FLASH HOLD) */
  GPIO_InitStruct.Pin = OCTA_FLASH_WP_Pin|OCTA_FLASH_HOLD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OCTA_FLASH_CS_Port, OCTA_FLASH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, OCTA_FLASH_WP_Pin|OCTA_FLASH_HOLD_Pin, GPIO_PIN_RESET);

  P1_DIO1.PORT = P1_DIO1_GPIO_Port;
  P1_DIO1.PIN =  P1_DIO1_Pin;
  P1_DIO2.PORT = P1_DIO2_GPIO_Port;
  P1_DIO2.PIN = P1_DIO2_Pin;
  P1_DIO3.PORT = P1_DIO3_GPIO_Port;
  P1_DIO3.PIN = P1_DIO3_Pin;
  P1_DIO4.PORT = P1_DIO4_GPIO_Port;
  P1_DIO4.PIN = P1_DIO4_Pin;
  P1_DIO5.PORT = P1_DIO5_GPIO_Port;
  P1_DIO5.PIN = P1_DIO5_Pin;
  P1_DIO6.PORT = P1_DIO6_GPIO_Port;
  P1_DIO6.PIN = P1_DIO6_Pin;

  P2_DIO1.PORT = P2_DIO1_GPIO_Port;
  P2_DIO1.PIN =  P2_DIO1_Pin;
  P2_DIO2.PORT = P2_DIO2_GPIO_Port;
  P2_DIO2.PIN = P2_DIO2_Pin;
  P2_DIO3.PORT = P2_DIO3_GPIO_Port;
  P2_DIO3.PIN = P2_DIO3_Pin;
  P2_DIO4.PORT = P2_DIO4_GPIO_Port;
  P2_DIO4.PIN = P2_DIO4_Pin;
  P2_DIO5.PORT = P2_DIO5_GPIO_Port;
  P2_DIO5.PIN = P2_DIO5_Pin;
  P2_DIO6.PORT = P2_DIO6_GPIO_Port;
  P2_DIO6.PIN = P2_DIO6_Pin;

  P3_DIO1.PORT = P3_DIO1_GPIO_Port;
  P3_DIO1.PIN =  P3_DIO1_Pin;
  P3_DIO2.PORT = P3_DIO2_GPIO_Port;
  P3_DIO2.PIN = P3_DIO2_Pin;
  P3_DIO3.PORT = P3_DIO3_GPIO_Port;
  P3_DIO3.PIN = P3_DIO3_Pin;
  P3_DIO4.PORT = P3_DIO4_GPIO_Port;
  P3_DIO4.PIN = P3_DIO4_Pin;
  P3_DIO5.PORT = P3_DIO5_GPIO_Port;
  P3_DIO5.PIN = P3_DIO5_Pin;
  P3_DIO6.PORT = P3_DIO6_GPIO_Port;
  P3_DIO6.PIN = P3_DIO6_Pin;

}