#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;

void StartDefaultTask(void const *argument);
void iim423xx_measurement(void);