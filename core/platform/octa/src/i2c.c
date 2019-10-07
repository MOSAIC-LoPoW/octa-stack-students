#include "i2c.h"

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */

 void common_I2C_Init(void)
 {
    common_I2C.Instance = I2C1;
    common_I2C.Init.Timing = 0x00101A26;
    common_I2C.Init.OwnAddress1 = 0;
    common_I2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    common_I2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    common_I2C.Init.OwnAddress2 = 0;
    common_I2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    common_I2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    common_I2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&common_I2C) != HAL_OK)
    {
      Error_Handler();
    }
    /**Configure Analogue filter 
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&common_I2C, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
      Error_Handler();
    }
    /**Configure Digital filter 
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&common_I2C, 0) != HAL_OK)
    {
      Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */

 }


/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
void P1_I2C_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  P1_I2C.Instance = I2C3;
  P1_I2C.Init.Timing = 0x00101A26;
  P1_I2C.Init.OwnAddress1 = 0;
  P1_I2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  P1_I2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  P1_I2C.Init.OwnAddress2 = 0;
  P1_I2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  P1_I2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  P1_I2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&P1_I2C) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&P1_I2C, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&P1_I2C, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

void P2_I2C_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  P2_I2C.Instance = I2C2;
  P2_I2C.Init.Timing = 0x00101A26;
  P2_I2C.Init.OwnAddress1 = 0;
  P2_I2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  P2_I2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  P2_I2C.Init.OwnAddress2 = 0;
  P2_I2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  P2_I2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  P2_I2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&P2_I2C) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&P2_I2C, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&P2_I2C, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
void P3_I2C_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  P3_I2C.Instance = I2C4;
  P3_I2C.Init.Timing = 0x00101A26;
  P3_I2C.Init.OwnAddress1 = 0;
  P3_I2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  P3_I2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  P3_I2C.Init.OwnAddress2 = 0;
  P3_I2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  P3_I2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  P3_I2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&P3_I2C) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&P3_I2C, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /**Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&P3_I2C, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}