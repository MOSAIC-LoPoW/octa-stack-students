#include "stm32l4xx_hal.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;

__weak void SystemClock_Config(void);