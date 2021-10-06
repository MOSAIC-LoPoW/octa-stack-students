#include "stm32l4xx_hal.h"

RTC_HandleTypeDef hrtc;

void RTC_Init(void);
void RTC_Set_Time_From_Unix(uint32_t UnixNum);
uint32_t RTC_Time_To_Unix();
void RTC_Print_DateTime();
void RTC_SetApplicationWakeupCallback(void (*callback));