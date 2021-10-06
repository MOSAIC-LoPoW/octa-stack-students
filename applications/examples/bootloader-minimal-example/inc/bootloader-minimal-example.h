#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;

void printWelcome(void);

#ifdef __cplusplus
}
#endif

#endif