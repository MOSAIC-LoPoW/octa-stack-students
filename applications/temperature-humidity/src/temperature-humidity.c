#include "temperature-humidity.h"
#include "sht31.h"

#define temp_hum_timer    3

osTimerId temp_hum_timer_id;
float SHTData[2];

int main(void)
{
  Initialize_Platform();
  
  //SHT
  setI2CInterface_SHT31(&common_I2C);
  SHT31_begin();

  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(temp_hum_Tim, temp_hum_measurement);
  temp_hum_timer_id = osTimerCreate(osTimer(temp_hum_Tim), osTimerPeriodic, NULL);
  osTimerStart(temp_hum_timer_id, temp_hum_timer * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void temp_hum_measurement(void)
{
  SHT31_get_temp_hum(SHTData);
  print_temp_hum();
}

void print_temp_hum(void)
{
  printINF("Temperature: %.2f degC \r\n", SHTData[0]);
  printINF("Humidity: %.2f %% \r\n", SHTData[1]);
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    osDelay(1);
  }
}