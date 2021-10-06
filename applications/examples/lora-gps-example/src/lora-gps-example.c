#include "lora-gps-example.h"
#include "fireflyx1.h"
#include "lorawan_stack.h"
#include "octa-flash.h"
#include "FreeRTOS.h"
#include "payloadparser.h"
#include "murata.h" // Change data rate and ADR in file shields/Murata-dualstack/inc/murata.h

#define IWDG_INTERVAL           5    //seconds
#define LORAWAN_INTERVAL        30   //seconds
#define MODULE_CHECK_INTERVAL   3600 //seconds

uint16_t LoRaWAN_Counter = 0;
uint8_t murata_init = 0;
uint64_t short_UID;
uint8_t murata_data_ready = 0;
float locationArray[3];
uint8_t locationArrayHex[32];
uint8_t hasGPSFix = 0;
gps_position_dd_t currentLocation={0,0,0};

uint8_t measurement_ongoing = 0;
uint8_t test_mode_active = 0;
uint8_t erase_ongoing = 0;
uint8_t BTN2presses = 0;
uint8_t NucleoBTN1presses = 0;
uint8_t rx[OCTA_FLASH_PAGE_SIZE];
uint8_t data[OCTA_FLASH_PAGE_SIZE];
uint8_t coordinate = 1;
uint8_t count = 0;

// The current configuration is stored here
struct octa_configuration current_octa_configuration = {0};

// Fallback configuration
struct octa_configuration default_octa_configuration = {.flash_index = 0, 
                                                        .dns_id = 0, 
                                                        .last_msg_acked = 0, 
                                                        .interval = 0, 
                                                        .multiplier = 0};

int main(void)
{
  Initialize_Platform();
  OCTA_FLASH_Initialize(&FLASH_SPI);

  //UNCOMMENT TO USE NEW DEFAULT OCTA CONFIG
  //OCTA_FLASH_eraseConfigSector();

  Get_Current_Config_From_Flash();

  // initial sensor interval & multiplier
  printINF("Sensor measurement interval: %d minutes\r\n", current_octa_configuration.interval);
  printINF("Amount of measurements per tx = %d \r\n\r\n", current_octa_configuration.multiplier);

  // LORAWAN
  murata_init = Murata_Initialize(short_UID);
  UART_SetApplicationCallback(&Dualstack_ApplicationCallback, (uint8_t)MURATA_CONNECTOR);

  if (murata_init)
  {
    printINF("Murata dualstack module init OK\r\n\r\n");
  }  
  // Firefly-GPS
  Firefly_Initialize();
  memset(locationArrayHex, 1, 32); // init location array to 0
 
  // TX MUTEX ensuring no transmits are happening at the same time
  osMutexDef(txMutex);
  txMutexId = osMutexCreate(osMutex(txMutex));
  
  osThreadDef(murata_rx_processing, murata_process_rx_response, osPriorityNormal, 0, 512);
  murata_rx_processing_handle = osThreadCreate(osThread(murata_rx_processing), NULL);

  // pass processing thread handle to murata driver
  Murata_SetProcessingThread(murata_rx_processing_handle);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, IWDG_INTERVAL * 1000);

  osTimerDef(loraWANTim, LoRaWAN_send);
  loraWANTimId = osTimerCreate(osTimer(loraWANTim), osTimerPeriodic, NULL);
  osTimerStart(loraWANTimId, LORAWAN_INTERVAL * 1000);

  osTimerDef(moduleCheckTim, check_modules);
  moduleCheckTimId = osTimerCreate(osTimer(moduleCheckTim), osTimerPeriodic, NULL);
  osTimerStart(moduleCheckTimId, MODULE_CHECK_INTERVAL * 1000);

  osThreadDef(offloadFlash, Offload_Flash, osPriorityNormal, 0, 512);
  offloadFlashTaskHandle = osThreadCreate(osThread(offloadFlash), NULL);

  osThreadDef(eraseFlash, Erase_Flash, osPriorityNormal, 0, 512);
  eraseFlashTaskHandle = osThreadCreate(osThread(eraseFlash), NULL);

  // Enable button interrupt for flash offloading
  B2_enable_interrupt();
  GPIO_SetApplicationCallback(&BTN2_Appcallback, (uint16_t)OCTA_BTN2_Pin);

  // Enable button interrupt for flash erasing
  Nucleo_BTN1_enable_interrupt();
  GPIO_SetApplicationCallback(&Nucleo_BTN1_Appcallback, (uint16_t)B1_Pin);

  //Join before starting the kernel
  Murata_LoRaWAN_Join();

  osKernelStart();

  while (1)
  { 
  }
}

void LoRaWAN_send(void const *argument)
{
  GPS_Read();

  if (murata_init)
  {
    uint8_t loraMessage[256];
    uint8_t i = 0;
    //uint16 counter to uint8 array (little endian)
    //counter (large) type byte
    loraMessage[i++] = TYPE_MESSAGE_COUNTER_LARGE;
    loraMessage[i++] = LoRaWAN_Counter;
    loraMessage[i++] = LoRaWAN_Counter >> 8;

    if(hasGPSFix == 1)
    {
      // add GPS coordinates
      printINF("GPS FIX, adding to payload\r\n");
      loraMessage[i++] = TYPE_GPS_LONG;
      float_union.fl = locationArray[0];  // LAT
      loraMessage[i++] = float_union.bytes.b1;
      loraMessage[i++] = float_union.bytes.b2;
      loraMessage[i++] = float_union.bytes.b3;
      loraMessage[i++] = float_union.bytes.b4;  
     
      float_union.fl = locationArray[1];  // LONG
      loraMessage[i++] = float_union.bytes.b1;
      loraMessage[i++] = float_union.bytes.b2;
      loraMessage[i++] = float_union.bytes.b3;
      loraMessage[i++] = float_union.bytes.b4;

      float_union.fl = locationArray[2];  // HDOP
      loraMessage[i++] = float_union.bytes.b1;
      loraMessage[i++] = float_union.bytes.b2;
      loraMessage[i++] = float_union.bytes.b3;
      loraMessage[i++] = float_union.bytes.b4;
    }

    //check if index is start of new sector (sector has 16 pages), if yes erase this sector
    if(current_octa_configuration.flash_index % 16 == 0)
    {
        printINF("Flash page %d is the start of sector %d, erasing this sector.\r\n", current_octa_configuration.flash_index, current_octa_configuration.flash_index/PAGES_PER_4KBSECTOR);
        OCTA_FLASH_erase4KBSectorFromPage(current_octa_configuration.flash_index);
        while(OCTA_FLASH_isWriteInProgress())
        {
            printDBG("Write in progress\r\n");
            HAL_Delay(50);
        }
    }
    //open current flash index
    OCTA_FLASH_open(current_octa_configuration.flash_index);
    printINF("Writing payload (%d databytes) to flash page %d\r\n", i, current_octa_configuration.flash_index);
    OCTA_FLASH_write(loraMessage, OCTA_FLASH_PAGE_SIZE);
    while(OCTA_FLASH_isWriteInProgress())
    {
        printDBG("Write in progress\r\n");
        HAL_Delay(50);
    }
    //index++
    current_octa_configuration.flash_index++;
    printDBG("New Flash Index: %d, updating config in flash\r\n", current_octa_configuration.flash_index);
    //update index in flash
    update_flash_configuration(&current_octa_configuration);

    osMutexWait(txMutexId, osWaitForever);
    if(!Murata_LoRaWAN_Send((uint8_t *)loraMessage, i)) 
    {  
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_BLED_Pin);
      HAL_Delay(100);
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_BLED_Pin);
      HAL_Delay(100);
      murata_init++;
      if(murata_init == 10)
        murata_init == 0;
    }
    else {
      murata_init = 1;
    }

    osDelay(3000);  //BLOCK TX MUTEX FOR 3s
    osMutexRelease(txMutexId);
    LoRaWAN_Counter++;
  }
  else {
    printINF("murata not initialized, not sending\r\n");
  }
}

void GPS_Read()
{
  uint8_t response[GPS_BUFFER_SIZE];
  uint8_t quality = Firefly_receive(response);
  if(quality==1)
  {   
      printINF("GPS FIX\r\n");
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
      HAL_Delay(100);
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
      HAL_Delay(100);
      setGPSCoordinates();
  }
  else
  {
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(100);
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(100);
      hasGPSFix=0;
      printINF("NO GPS FIX\r\n");
  }
}

void setGPSCoordinates(void)
{
	hasGPSFix=1;
	currentLocation=gps_get_position_dd();
	locationArray[0]=currentLocation.latitude;
	locationArray[1]=currentLocation.longitude;
	locationArray[2]=currentLocation.hdop;
  printINF("LOCATION:  lat: %f, long: %f, hdop: %f \r\n", locationArray[0], locationArray[1], locationArray[2]);
}

void Get_Current_Config_From_Flash(void)
{
  // GET CURRENT CONFIGURATION
  printINF("Getting current configuration from flash\r\n");
  if(OCTA_FLASH_openConfigSector()!= OCTA_FLASH_OK)
  {
    printERR("Flash open NOK, setting default config\r\n");
    current_octa_configuration = default_octa_configuration;
    return;
  }
  static uint8_t configurationbuffer[256] = {0xff};
  uint32_t retval = 0;
  retval = OCTA_FLASH_read(configurationbuffer, OCTA_FLASH_PAGE_SIZE);
  if(retval != OCTA_FLASH_PAGE_SIZE)
  {
    printERR("Flash operation NOK: %d, setting default config\r\n", retval);
    current_octa_configuration = default_octa_configuration;
    return;
  }
  if(parse_octa_configuration(&current_octa_configuration, configurationbuffer, CONFIG_BUFFER_SIZE)==PARSER_OK)
  {
    printINF("Parsing config from flash OK\r\n");
    return;
  }
  else
  {
    printERR("UNABLE TO PARSE CONFIGURATION FROM FLASH, setting default config\r\n");
    current_octa_configuration = default_octa_configuration;
    return;
  }
}

void B1_enable_interrupt()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = OCTA_BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OCTA_BTN1_GPIO_Port, &GPIO_InitStruct);

  //enable the interrupt
  HAL_NVIC_SetPriority(EXTI0_IRQn, 8, 0);		
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void B2_enable_interrupt()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OCTA_BTN2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(OCTA_BTN2_GPIO_Port, &GPIO_InitStruct);

    //enable the interrupt
    HAL_NVIC_SetPriority(EXTI1_IRQn, 8, 0);		
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void Nucleo_BTN1_enable_interrupt()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

    //enable the interrupt
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 8, 0);		
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void BTN2_Appcallback(void)
{
  if(!measurement_ongoing && !erase_ongoing)
  {
    if(!BTN2presses)
    {
      BTN2presses++;
      RTOS_Send_Notification(offloadFlashTaskHandle);
    }  
    else if(BTN2presses == 1)
    {
      BTN2presses++;
      printINF("BTN2 pressed twice\r\n");  
    }
    else
      BTN2presses = 0;
  } 
}

void Nucleo_BTN1_Appcallback(void)
{
  if(!measurement_ongoing && !erase_ongoing)
  {
    if(!NucleoBTN1presses)
    {
      NucleoBTN1presses++;
      RTOS_Send_Notification(eraseFlashTaskHandle);
    }  
    else if(NucleoBTN1presses == 4)
    {
      NucleoBTN1presses++;
      printINF("Nucleo BTN pressed 5 times\r\n");
    }
    else
    {
      NucleoBTN1presses++;
    } 
  } 
}

void Offload_Flash()
{
  uint32_t startProcessing;
  while (1)
  {
    // Wait to be notified that the transmission is complete.  Note the first
    //parameter is pdTRUE, which has the effect of clearing the task's notification
    //value back to 0, making the notification value act like a binary (rather than
    //a counting) semaphore.
    startProcessing = ulTaskNotifyTake(pdTRUE, osWaitForever);
    if (startProcessing == 1)
    {
      // The transmission ended as expected.
      printDBG("Press BTN2 again within 5 sec to offload flash\r\n");
      uint8_t i = 0;

      for (i=0; i<5; i++)
      {
        if(BTN2presses == 2)
        { 
          printINF("*****OFFLOADING FLASH*****\r\n");
          
         printINF("Stopping lorawan timer\r\n");
         osTimerStop(loraWANTimId);

          uint32_t index;
          printINF("Offloading a total of %d pages\r\n", current_octa_configuration.flash_index);
          for(index=0; index<current_octa_configuration.flash_index; index++)
          {
            OCTA_FLASH_open(index);
            uint8_t buffer[OCTA_FLASH_PAGE_SIZE];
            OCTA_FLASH_read(buffer, OCTA_FLASH_PAGE_SIZE);
            uint16_t byteindex;
            unsigned char* result;
            result = bin_to_strhex((unsigned char *)buffer, OCTA_FLASH_PAGE_SIZE, &result);
            printINF("Flash page %d: %s\r\n", index, result);
            free(result); 
          }
          printINF("Flash offloaded\r\n");
          
          printINF("Restarting lorawan Timer\r\n");
          osTimerStart(loraWANTimId, 30 * 1000);
          break;
        }
        else
        {
          printDBG("...\r\n");
          osDelay(1000);
        }
        
      }
      BTN2presses = 0;
    }
    else
    {
      printERR("Notification NOK\r\n");
    }
    osDelay(1);
  }
  osThreadTerminate(NULL);
}

void Erase_Flash()
{
  uint32_t startProcessing;
  while (1)
  {
    // Wait to be notified that the transmission is complete.  Note the first
    //parameter is pdTRUE, which has the effect of clearing the task's notification
    //value back to 0, making the notification value act like a binary (rather than
    //a counting) semaphore.
    startProcessing = ulTaskNotifyTake(pdTRUE, osWaitForever);
    if (startProcessing == 1)
    {
      // The transmission ended as expected.
      printDBG("Press Nucleo BTN1 5 times within 5 sec to erase flash\r\n");
      uint8_t i = 0;

      for (i=0; i<5; i++)
      {
        if(NucleoBTN1presses > 4)
        { 
          printINF("*****ERASING FLASH*****\r\n");
          erase_ongoing = 1;
          printINF("Stopping lorawan timer\r\n");
          osTimerStop(loraWANTimId);

          if(OCTA_FLASH_eraseFullFlash()==OCTA_FLASH_OK)
            printINF("Flash Erase CMD OK, actual erase takes a few minutes. Red LED will blink during erase\r\n");
          else
            printERR("Flash Erase CMD NOK\r\n");
          
          while(OCTA_FLASH_isWriteInProgress())
          {
            IWDG_feed(NULL);
            HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
            HAL_Delay(500);
          }
          HAL_GPIO_WritePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin, GPIO_PIN_SET);
          printINF("Erase finished\r\n");

          Get_Current_Config_From_Flash();
          
          erase_ongoing = 0;
          printINF("Restarting lorawan Timer\r\n");
          osTimerStart(loraWANTimId, 30 * 1000);
          break;
        }
        else
        {
          printDBG("...\r\n");
          osDelay(1000);
        }
      }
      printINF("Resetting Nucleo BTN1 presses\r\n");
      NucleoBTN1presses = 0;
    }
    else
    {
      printERR("Notification NOK\r\n");
    }
    osDelay(1);
  }
  osThreadTerminate(NULL);
}

void check_modules(void const *argument)
{
  printINF("checking the status of the modules\r\n");
  if (!murata_init)
  {
    // LORAWAN
    murata_init = Murata_Initialize(short_UID);
    Murata_toggleResetPin();
  }
}

void murata_process_rx_response(void const *argument)
{
  uint32_t startProcessing;
  while (1)
  {
    // Wait to be notified that the transmission is complete.  Note the first
    //parameter is pdTRUE, which has the effect of clearing the task's notification
    //value back to 0, making the notification value act like a binary (rather than
    //a counting) semaphore.
    startProcessing = ulTaskNotifyTake(pdTRUE, osWaitForever);
    if (startProcessing == 1)
    {
      // The transmission ended as expected.
      while(murata_data_ready)
      {
        printDBG("processing murata fifo\r\n");
        murata_data_ready = !Murata_process_fifo();
        osDelay(50);
      }
    }
    else
    {
    }
    osDelay(1);
  }
  osThreadTerminate(NULL);
}

void Dualstack_ApplicationCallback(void)
{
  murata_data_ready = 1;
}

unsigned char * bin_to_strhex(const unsigned char *bin, unsigned int binsz, unsigned char **result)
{
  unsigned char     hex_str[]= "0123456789abcdef";
  unsigned int      i;

  if (!(*result = (unsigned char *)malloc(binsz * 2 + 1)))
    return (NULL);

  (*result)[binsz * 2] = 0;

  if (!binsz)
    return (NULL);

  for (i = 0; i < binsz; i++)
    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }
  return (*result);
}

// uint8_t Write_To_Sector(uint8_t dataIndex,uint8_t *locationA)
// {
//      // if(dataIndex <= 255)
//      // {
//         //check if index is start of new sector (sector has 16 pages), if yes erase this sector
//         if(current_octa_configuration.flash_index % 16 == 0)
//         {
//             printINF("Flash page %d is the start of sector %d, erasing this sector.\r\n", current_octa_configuration.flash_index, current_octa_configuration.flash_index/PAGES_PER_4KBSECTOR);
//             OCTA_FLASH_erase4KBSectorFromPage(current_octa_configuration.flash_index);
//             while(OCTA_FLASH_isWriteInProgress())
//             {
//                 printDBG("Write in progress\r\n");
//                 HAL_Delay(50);
//             }
//         }
//         //open current flash index
//         OCTA_FLASH_open(current_octa_configuration.flash_index);
//         printINF("Writing GPS FIX (%d databytes) to flash page %d\r\n", dataIndex, current_octa_configuration.flash_index);
//         OCTA_FLASH_write(locationA, OCTA_FLASH_PAGE_SIZE);

//         while(OCTA_FLASH_isWriteInProgress())
//         {
//             printDBG("Write in progress\r\n");
//             HAL_Delay(50);
//         }


//         // READ FLASH TO CHECK
//         OCTA_FLASH_open(current_octa_configuration.flash_index);
//         printf("\n\r \n\r ");

//         OCTA_FLASH_read(rx,OCTA_FLASH_PAGE_SIZE);
//         printINF("READING PAGE %d: \n\r", current_octa_configuration.flash_index);

//         for(uint16_t i = 0; i < 252; i=i+12)
//         {
//           printDBG("GPS LOCATION %d :", coordinate); 
//           printDBG("LAT: %d %d %d %d ", rx[i], rx[i+1], rx[i+2], rx[i+3]); 
//           printDBG("LONG: %d %d %d %d ", rx[i+4], rx[i+5], rx[i+6], rx[i+7]);
//           printDBG("HDOP: %d %d %d %d \n\r", rx[i+8], rx[i+9], rx[i+10], rx[i+11]);
//           coordinate++;
//         }


//         current_octa_configuration.flash_index++;
//         printDBG("New Flash Index: %d, updating config in flash\r\n", current_octa_configuration.flash_index);
//         //update index in flash
//         update_flash_configuration(&current_octa_configuration);
//     //  }
//     //  else {
//       //    printERR("Message too large to store in flash\r\n");
//       //}
// }