#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;

void StartDefaultTask(void const *argument);
void RTC_Wakeup(void);
void Application_ReInit(void);
void Application_DeInit(void);