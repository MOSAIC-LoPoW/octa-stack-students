
#include "nb-iot-example.h"
#include "SARAN21X.h"
#include "dns.h"
 
#define IWDG_INTERVAL             5     //seconds
#define NBIOT_INTERVAL            30    //seconds

uint8_t nb_iot_init = 0;
uint8_t DNS_success = 0;

char* nb_iot_address;
char* Msg ="masala est au bureau a beacon,masala est au bureau a beacon,masala est au bureau a beacon,masala est au bureau a beacon";

int main(void)
{
  Initialize_Platform();

  // NB-IoT
  nb_iot_init = SARAN21X_Initialize();
  // Init the module's radio and network settings
  nb_iot_init = nb_iot_init && SARAN21X_init_module((const char *)NB_IOT_DNS_SERVER, 53);

  if (nb_iot_init)
  {
    SARAN21X_setApplicationRxCallback(&nb_iot_callback);
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
    if (!DNS_success)
    {
      uint16_t length;
      uint8_t* nbIotMessage = generateRequest(NB_IOT_SERVER_NAME, &length);
      printINF("Sending DNS request for %s.\r\n", NB_IOT_SERVER_NAME);
      nb_iot_init = SARAN21X_send(nbIotMessage, length);
    }
    else
    {
      printINF("Sending narrowband IoT message \"%s\" with size :%d \r\n", Msg, strlen(Msg));
      nb_iot_init = SARAN21X_send(Msg, strlen(Msg));
    }   
  }
  else{
    printINF("nb-iot not initialized, not sending.\r\n");
  }
}

void nb_iot_callback (uint8_t *buffer, uint16_t len) 
{
  printDBG("\r\n");
  #if DEBUG
    for (int i = 0; i < len; ++i) {
      printf("%02x", buffer[i]);
    }
    printf("\r\n");
  #endif

  if(!DNS_success )
  {
    nb_iot_address = parseReply(buffer, len);
    if (nb_iot_address) {
      printINF("Reply received from DNS server. Address received: %s\r\n", nb_iot_address);
      DNS_success = 1;
      SARAN21X_set_server_parameters(nb_iot_address,4460);
    } else {
      printINF("DNS reply received. reply did not contain a valid IP address\r\n");
    }
  }
  else
  {
    printINF("Downlink received: %s\r\n", buffer);
  }
}