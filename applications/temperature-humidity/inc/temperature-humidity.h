#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;

void StartDefaultTask(void const *argument);
void temp_hum_measurement(void);
void print_temp_hum(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */