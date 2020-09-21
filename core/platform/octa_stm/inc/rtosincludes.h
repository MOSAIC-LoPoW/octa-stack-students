#include "cmsis_os.h"

osThreadId rtosprintHandle;

void RTOS_Send_Notification(osThreadId threadid);
void RTOS_Print_Stats_Thread(void const *argument);
void RTOS_print_stats(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char pcTaskName);