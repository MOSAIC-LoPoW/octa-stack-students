#include "spi.h"
#include "platform.h"

void P3_SPI_Init(void)
{
  P3_SPI.Instance = SPI1;
  P3_SPI.Init.Mode = SPI_MODE_MASTER;
  P3_SPI.Init.Direction = SPI_DIRECTION_2LINES;
  P3_SPI.Init.DataSize = SPI_DATASIZE_4BIT;
  P3_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  P3_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  P3_SPI.Init.NSS = SPI_NSS_SOFT;
  P3_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  P3_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  P3_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
  P3_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  P3_SPI.Init.CRCPolynomial = 7;
  P3_SPI.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  P3_SPI.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&P3_SPI) != HAL_OK)
  {
    Error_Handler();
  }
}

void P2_SPI_Init(void)
{
  P2_SPI.Instance = SPI2;
  P2_SPI.Init.Mode = SPI_MODE_MASTER;
  P2_SPI.Init.Direction = SPI_DIRECTION_2LINES;
  P2_SPI.Init.DataSize = SPI_DATASIZE_4BIT;
  P2_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  P2_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  P2_SPI.Init.NSS = SPI_NSS_SOFT;
  P2_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  P2_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  P2_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
  P2_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  P2_SPI.Init.CRCPolynomial = 7;
  P2_SPI.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  P2_SPI.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&P2_SPI) != HAL_OK)
  {
    Error_Handler();
  }
}

void P1_SPI_Init(void)
{
  P1_SPI.Instance = SPI3;
  P1_SPI.Init.Mode = SPI_MODE_MASTER;
  P1_SPI.Init.Direction = SPI_DIRECTION_2LINES;
  P1_SPI.Init.DataSize = SPI_DATASIZE_4BIT;
  P1_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  P1_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  P1_SPI.Init.NSS = SPI_NSS_SOFT;
  P1_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  P1_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  P1_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
  P1_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  P1_SPI.Init.CRCPolynomial = 7;
  P1_SPI.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  P1_SPI.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&P1_SPI) != HAL_OK)
  {
    Error_Handler();
  }
}

void FLASH_SPI_Init(void)
{
  FLASH_SPI.Instance = SPI1;
  FLASH_SPI.Init.Mode = SPI_MODE_MASTER;
  FLASH_SPI.Init.Direction = SPI_DIRECTION_2LINES;
  FLASH_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
  FLASH_SPI.Init.CLKPolarity = SPI_POLARITY_LOW;
  FLASH_SPI.Init.CLKPhase = SPI_PHASE_1EDGE;
  FLASH_SPI.Init.NSS = SPI_NSS_SOFT;
  FLASH_SPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  FLASH_SPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
  FLASH_SPI.Init.TIMode = SPI_TIMODE_DISABLE;
  FLASH_SPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  FLASH_SPI.Init.CRCPolynomial = 7;
  FLASH_SPI.Init.CRCLength = SPI_CRC_LENGTH_8BIT;//SPI_CRC_LENGTH_DATASIZE;
  FLASH_SPI.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;//SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&FLASH_SPI) != HAL_OK)
  {
     Error_Handler();
  }
}