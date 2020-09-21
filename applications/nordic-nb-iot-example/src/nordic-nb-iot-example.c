
#include "nordic-nb-iot-example.h"
#include "NORDIC9160.h"
#include "dns.h"
 
#define IWDG_INTERVAL             5     //seconds
#define NBIOT_INTERVAL            10    //seconds

uint8_t nb_iot_init = 0;
char* nb_iot_address;

int main(void)
{
  Initialize_Platform();

  // NB-IoT
  nb_iot_init = NORDIC9160_Initialize();
  // Init the module's radio and network settings
  nb_iot_init = nb_iot_init && NORDIC9160_init_module((const char *)NB_IOT_SERVER_ADDRESS, NB_IOT_SERVER_PORT);

  if (nb_iot_init)
  {
    NORDIC9160_setApplicationRxCallback(&nb_iot_callback);
    printINF("NB-IoT module init OK\r\n\r\n");
  }

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, IWDG_INTERVAL * 1000);
  
  //run the NB_IoT_Send function every 30 seconds
  osTimerDef(nbIotTim, NB_IoT_send);
  nbIotTimId = osTimerCreate(osTimer(nbIotTim), osTimerPeriodic, NULL);
  osTimerStart(nbIotTimId, NBIOT_INTERVAL * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void NB_IoT_send(void const *argument)
{
  if (nb_iot_init)
  {
    uint8_t Msg[20] = {0};
    uint8_t index = 0;

    Msg[index++] = TYPE_NB_IOT_IMSI; //21 dec
    Msg[index++] = imsi_number.bytes.b1;
    Msg[index++] = imsi_number.bytes.b2;
    Msg[index++] = imsi_number.bytes.b3;
    Msg[index++] = imsi_number.bytes.b4;
    Msg[index++] = imsi_number.bytes.b5;
    Msg[index++] = imsi_number.bytes.b6;
    Msg[index++] = imsi_number.bytes.b7;
    Msg[index++] = imsi_number.bytes.b8;
 
    printINF("Sending narrowband IoT message with size %d \r\n", index);
    nb_iot_init = NORDIC9160_send(Msg, index);
  }
  else
  {
    printINF("nb-iot not initialized, not sending.\r\n");
  }
}

void nb_iot_callback (uint8_t *buffer, uint16_t len) 
{
    printINF("Application callback: downlink received: %s\r\n", buffer);
}