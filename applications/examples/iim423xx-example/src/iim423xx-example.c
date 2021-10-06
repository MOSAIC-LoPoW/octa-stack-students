#include "iim423xx-example.h"
#include "iim423xx.h"

#define accelerometer_timer    5

osTimerId accelerometer_tim_id;
float co2_ppm, temperature, relative_humidity;

int main(void)
{
  Initialize_Platform();

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(accelerometer_tim, iim423xx_measurement);
  accelerometer_tim_id = osTimerCreate(osTimer(accelerometer_tim), osTimerPeriodic, NULL);
  osTimerStart(accelerometer_tim_id, accelerometer_timer * 1000);

  IIM423xx_Initialize();

  IIM423xx_setup();

  osKernelStart();

  while (1)
  {
  }
}

void iim423xx_measurement(void)
{
    GetDataFromInvDevice();
}
