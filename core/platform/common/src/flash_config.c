#include "payloadformat.h"
#include "octa-flash.h"
#include "datatypes.h"
#include "flash_config.h"

/*
***********************************
* Flash configuration (first page *
***********************************
* interval & multiplier
* last msg acked
* dns id
* current flash index (page?)
*/
uint8_t update_flash_configuration(struct octa_configuration* toFlash)
{
  int retval = 0;
  // write to flash as octa payload
  uint8_t flashBuffer[OCTA_FLASH_PAGE_SIZE] = {0xff};
  // read current flash
  if(OCTA_FLASH_openConfigSector()!= OCTA_FLASH_OK)
  {
    printERR("Flash open NOK\r\n");
  }

  retval = OCTA_FLASH_read(flashBuffer, OCTA_FLASH_PAGE_SIZE);
  if(retval != OCTA_FLASH_PAGE_SIZE)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  printDBG("Current flash configuration:\r\n");
  #if DEBUG
    for (uint16_t i = 0; i < OCTA_FLASH_PAGE_SIZE; ++i) {
      printf("%02x", flashBuffer[i]);
    }
    printf("\r\n");
  #endif

  uint16_t index = 0;
  // interval & multiplier
  flashBuffer[index++] = (uint8_t)TYPE_DOWNLINK_INTERVAL_MULTIPLIER_UPDATE;
  flashBuffer[index++] = toFlash->interval;
  flashBuffer[index++] = toFlash->multiplier;

  // last acked msg
  flashBuffer[index++] = (uint8_t)TYPE_DOWNLINK_ACK;
  int32LittleEndian.integer = toFlash->last_msg_acked;
  flashBuffer[index++] = int32LittleEndian.byte[0];
  flashBuffer[index++] = int32LittleEndian.byte[1];
  flashBuffer[index++] = int32LittleEndian.byte[2];
  flashBuffer[index++] = int32LittleEndian.byte[3];

  // dns id
  flashBuffer[index++] = (uint8_t)TYPE_DOWNLINK_DNS_UPDATE;
  flashBuffer[index++] = toFlash->dns_id;

  // current flash index page
  flashBuffer[index++] = (uint8_t)TYPE_CURRENT_FLASH_INDEX;
  uint32LittleEndian.integer = toFlash->flash_index;
  flashBuffer[index++] = uint32LittleEndian.byte[0];
  flashBuffer[index++] = uint32LittleEndian.byte[1];
  flashBuffer[index++] = uint32LittleEndian.byte[2];
  flashBuffer[index++] = uint32LittleEndian.byte[3];

  retval = OCTA_FLASH_eraseConfigSector();
  if(retval != OCTA_FLASH_OK)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  while(OCTA_FLASH_isWriteInProgress())
  {
        printDBG("Write in progress\r\n");
        HAL_Delay(50);
  }
  uint8_t rxbuffer[OCTA_FLASH_PAGE_SIZE] = {0xff};
  retval = OCTA_FLASH_read(rxbuffer, OCTA_FLASH_PAGE_SIZE);
  if(retval != OCTA_FLASH_PAGE_SIZE)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  printDBG("Flash after erase\r\n");
  #if DEBUG
    for (uint16_t i = 0; i < OCTA_FLASH_PAGE_SIZE; ++i) {
      printf("%02x", rxbuffer[i]);
    }
    printf("\r\n");
  #endif

  printDBG("Writing updated configuration to flash:\r\n");
  #if DEBUG
    for (uint16_t i = 0; i < OCTA_FLASH_PAGE_SIZE; ++i) {
      printf("%02x", flashBuffer[i]);
    }
    printf("\r\n");
  #endif
  
  retval =  OCTA_FLASH_write(flashBuffer, OCTA_FLASH_PAGE_SIZE);
  HAL_Delay(50);
  if(retval != OCTA_FLASH_PAGE_SIZE)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  while(OCTA_FLASH_isWriteInProgress())
  {
        printDBG("Write in progress\r\n");
        HAL_Delay(50);
  }
  memset(flashBuffer, 0, OCTA_FLASH_PAGE_SIZE);
  retval = OCTA_FLASH_read(flashBuffer, OCTA_FLASH_PAGE_SIZE);
  if(retval != OCTA_FLASH_PAGE_SIZE)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  printDBG("Flash after new config\r\n");
  #if DEBUG
    for (uint16_t i = 0; i < OCTA_FLASH_PAGE_SIZE; ++i) {
      printf("%02x", flashBuffer[i]);
    }
    printf("\r\n");
  #endif
  return retval;
}