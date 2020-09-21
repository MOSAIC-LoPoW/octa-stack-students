#include "octa-flash-example.h"
#include "octa-flash.h"

int main(void)
{
  Initialize_Platform();

  OCTA_FLASH_Initialize(&FLASH_SPI);
  
  int retval = 0;
  retval = OCTA_FLASH_erase4KBSectorFromPage(0);
  if(retval != OCTA_FLASH_OK)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }
  //RW test  
  uint8_t RWsuccess = OCTA_FLASH_TestRW(); 

  //erase sector from page 7 should erase the entire first user sector, also the first 3 pages used in the test, test should work again though
  retval = OCTA_FLASH_erase4KBSectorFromPage(7);
  if(retval != OCTA_FLASH_OK)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }

  //try again
  RWsuccess = OCTA_FLASH_TestRW();

  //write something to page 20 -> part of sector 2
  OCTA_FLASH_open(20);
  uint8_t buffer[OCTA_FLASH_PAGE_SIZE];
  memset(buffer, 20, 256);
  OCTA_FLASH_write(buffer, OCTA_FLASH_PAGE_SIZE);
  //read to check
  uint8_t rx[OCTA_FLASH_PAGE_SIZE];
  OCTA_FLASH_read(rx,OCTA_FLASH_PAGE_SIZE);
  printDBG("sector 20: \r\n");
  for(uint16_t i = 0; i<256; i++)
      printf("%d ", rx[i]); 
  printf("\r\n");

  //erase starting from page 16, should not affect RWtest when running again (==sector 2)
  retval = OCTA_FLASH_erase4KBSectorFromPage(16);
  if(retval != OCTA_FLASH_OK)
  {
    printERR("Flash operation NOK: %d\r\n", retval);
  }

  //This should work again
  RWsuccess = OCTA_FLASH_TestRW();

  //This should be 255 now, sector 2 is erased
  OCTA_FLASH_open(20);
  OCTA_FLASH_read(rx,OCTA_FLASH_PAGE_SIZE);
  printDBG("sector 20: \r\n");
  for(uint16_t i = 0; i<256; i++)
      printf("%d ", rx[i]); 
  printf("\r\n");

  if(RWsuccess)
      printINF("octa-flash RW test OK\r\n");
  else
      printINF("octa-flash RW test NOK\r\n");
  while (1)
  {
    IWDG_feed(NULL);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    if(RWsuccess)
    {
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
      HAL_Delay(1000);
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
      HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
      HAL_Delay(1000);
      HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    }
  }
}