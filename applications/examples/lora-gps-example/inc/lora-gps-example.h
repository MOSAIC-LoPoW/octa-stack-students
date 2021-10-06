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
osTimerId dash7TimId;
osTimerId moduleCheckTimId;
osTimerId gpsTimId;
osTimerId sensor_measurementTimId;
osMutexId txMutexId;
osMutexId murata_rx_process_mutex_id;
osThreadId offloadFlashTaskHandle;
osThreadId eraseFlashTaskHandle;


void IWDG_feed(void const *argument);
void LoRaWAN_send(void const *argument);
void Dash7_send(void const *argument);
void check_modules(void const *argument);
void murata_process_rx_response(void const *argument);
void Dualstack_ApplicationCallback(void);

// GPS functions
void setGPSCoordinates();
void GPS_Read();

// Flash functions
void Offload_Flash(void);
void Erase_Flash(void);
void Get_Current_Config_From_Flash(void);
void BTN1_Appcallback(void);
void BTN2_Appcallback(void);
void Nucleo_BTN1_Appcallback(void);
unsigned char * bin_to_strhex(const unsigned char *bin, unsigned int binsz, unsigned char **result);
uint8_t Write_To_Sector(uint8_t index, uint8_t *locArray);




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
