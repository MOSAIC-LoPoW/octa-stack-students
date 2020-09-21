
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;
osTimerId nbIotTimId;
osTimerId moduleCheckTimId;

osMutexId txMutexId;

void IWDG_feed(void const *argument);
void NB_IoT_send(void const *argument);
void SARAN_process_rx_response(void const *argument);
void nb_iot_callback(uint8_t *buffer, uint16_t len);

#ifdef __cplusplus
}
#endif
#endif
