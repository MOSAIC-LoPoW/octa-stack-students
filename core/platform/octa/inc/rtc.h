#include "stm32l4xx_hal.h"

RTC_HandleTypeDef hrtc;

void RTC_Init(void);
void RTC_SetApplicationWakeupCallback(void (*callback));