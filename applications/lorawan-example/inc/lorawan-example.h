#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

osThreadId defaultTaskHandle;
osThreadId murata_rx_processing_handle;
osTimerId iwdgTimId;
osTimerId loraWANTimId;
osTimerId moduleCheckTimId;
osMutexId txMutexId;
osMutexId murata_rx_process_mutex_id;

void IWDG_feed(void const *argument);
void LoRaWAN_send(void const *argument);
void check_modules(void const *argument);
void murata_process_rx_response(void const *argument);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */