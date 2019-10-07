#include "stm32l4xx_hal.h"

I2C_HandleTypeDef common_I2C;
I2C_HandleTypeDef P1_I2C;
I2C_HandleTypeDef P2_I2C;
I2C_HandleTypeDef P3_I2C;

void common_I2C_Init(void);
void P1_I2C_Init(void);
void P2_I2C_Init(void);
void P3_I2C_Init(void);
