#include "murata-dualstack-example-ns.h"
#include "murata.h"

#define IWDG_INTERVAL           5    //seconds
#define LORAWAN_INTERVAL        60   //seconds
#define DASH7_INTERVAL          20  //seconds
#define MODULE_CHECK_INTERVAL   3600 //seconds

uint16_t LoRaWAN_Counter = 0;
uint16_t DASH7_Counter = 0;
uint8_t murata_init = 0;
uint64_t short_UID;
uint8_t murata_data_ready = 0;

int main(void)
{
  Initialize_Platform();
  
  // LORAWAN
  murata_init = Murata_Initialize(short_UID);
  UART_SetApplicationCallback(&Dualstack_ApplicationCallback, (uint8_t)MURATA_CONNECTOR);

  if (murata_init)
  {
    printINF("Murata dualstack module init OK\r\n\r\n");
  }
  
  /* Infinite loop */
  uint8_t counter = 0;
  uint8_t use_lora = 1;
  
  while (1)
  { 
    IWDG_feed(NULL);

    if(murata_data_ready)
    {
      printINF("processing murata fifo\r\n");
      murata_data_ready = !Murata_process_fifo();
    }
    
    // SEND 5 D7 messages, every 10 sec.
    // Afterwards, send 3 LoRaWAN messages, every minute
    if(DASH7_Counter<5)
    {
      if(counter==DASH7_INTERVAL)
      {
        Dash7_send(NULL);
        counter = 0;
      }
    }
    else
    { 
      if(LoRaWAN_Counter == 0)
        Murata_LoRaWAN_Join();
      if(LoRaWAN_Counter<3)
      {
        if (counter == LORAWAN_INTERVAL)
        {
          LoRaWAN_send(NULL);
          counter = 0;
        }
      }
      if(LoRaWAN_Counter == 3)
      {
        //reset counters to restart flow
        DASH7_Counter = 0;
        LoRaWAN_Counter = 0;
      }
    }
   
    counter++;
    HAL_Delay(1000);
  }
}

void LoRaWAN_send(void const *argument)
{
  if (murata_init)
  {
    uint8_t loraMessage[5];
    uint8_t i = 0;
    //uint16 counter to uint8 array (little endian)
    //counter (large) type byte
    loraMessage[i++] = 0x14;
    loraMessage[i++] = LoRaWAN_Counter;
    loraMessage[i++] = LoRaWAN_Counter >> 8;
    if(!Murata_LoRaWAN_Send((uint8_t *)loraMessage, i))
    {
      murata_init++;
      if(murata_init == 10)
        murata_init == 0;
    }
    else
    {
      murata_init = 1;
    }
    LoRaWAN_Counter++;
  }
  else{
    printINF("murata not initialized, not sending\r\n");
  }
}

void Dash7_send(void const *argument)
{
  if (murata_init)
  {
    uint8_t dash7Message[5];
    uint8_t i = 0;
    //uint16 counter to uint8 array (little endian)
    //counter (large) type byte
    dash7Message[i++] = 0x14;
    dash7Message[i++] = DASH7_Counter;
    dash7Message[i++] = DASH7_Counter >> 8;
    if(!Murata_Dash7_Send((uint8_t *)dash7Message, i))
    {
      murata_init++;
      if(murata_init == 10)
        murata_init == 0;
    }
    else
    {
      murata_init = 1;
    }
    DASH7_Counter++;
  }
  else{
    printINF("murata not initialized, not sending\r\n");
  }
}

void Dualstack_ApplicationCallback(void)
{
  murata_data_ready = 1;
}