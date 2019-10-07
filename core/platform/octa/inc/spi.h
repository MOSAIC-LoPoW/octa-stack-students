#include "stm32l4xx_hal.h"

SPI_HandleTypeDef P1_SPI;
SPI_HandleTypeDef P2_SPI;
SPI_HandleTypeDef P3_SPI;
SPI_HandleTypeDef FLASH_SPI;

void P1_SPI_Init(void);
void P2_SPI_Init(void);
void P3_SPI_Init(void);
void FLASH_SPI_Init(void);