#include "stm32l4xx_hal.h"

IWDG_HandleTypeDef hiwdg;

void OCTA_IWDG_Init(void);
void IWDG_feed(void const *argument);
