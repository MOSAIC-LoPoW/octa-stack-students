#include "octa-stm-mini-example.h"
#include "octa-flash.h"
#include "iim423xx.h"
#include "NORDIC9160.h"

#define accelerometer_timer    5

osTimerId accelerometer_tim_id;
uint8_t nb_iot_init = 0;

int main(void)
{
  Initialize_Platform();

  //IIM423xx
  IIM423xx_Initialize();

  IIM423xx_setup();

  HAL_GPIO_WritePin(OCTA_FLASH_VCC_SWITCH_Port, OCTA_FLASH_VCC_SWITCH_Pin, GPIO_PIN_SET);
  OCTA_FLASH_Initialize(&FLASH_SPI);
  OCTA_FLASH_erase4KBSectorFromPage(0);
  OCTA_FLASH_TestRW();

  //NB-IoT
  nb_iot_init = NORDIC9160_Initialize();
  // Init the module's radio and network settings
  nb_iot_init = nb_iot_init && NORDIC9160_init_module((const char *)NB_IOT_SERVER_ADDRESS, NB_IOT_SERVER_PORT);

  if (nb_iot_init)
  {
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    //NORDIC9160_setApplicationRxCallback(&nb_iot_callback);
    printINF("NB-IoT module init OK\r\n\r\n");
  }
  else
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);

  // feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(accelerometer_tim, iim423xx_measurement);
  accelerometer_tim_id = osTimerCreate(osTimer(accelerometer_tim), osTimerPeriodic, NULL);
  osTimerStart(accelerometer_tim_id, accelerometer_timer * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void iim423xx_measurement(void)
{
    GetDataFromInvDevice();
}