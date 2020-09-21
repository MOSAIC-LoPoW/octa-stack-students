#include "lightsensor.h"
#include "TCS3472.h"

#define lightsensor_timer    3
#define colorsensor_timer    3

osTimerId lightsensor_timer_id;
osTimerId colorsensor_timer_id;

int main(void)
{
  Initialize_Platform();

  osMutexDef(txMutex);
  i2cMutexId = osMutexCreate(osMutex(txMutex));

  /* USER CODE BEGIN 2 */
  TC3472_init(&common_I2C);
  
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  /* USER CODE END RTOS_TIMERS */
  osTimerDef(lightsensor_Tim, lightsensor_measurement);
  lightsensor_timer_id = osTimerCreate(osTimer(lightsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(lightsensor_timer_id, lightsensor_timer * 1000);

  osTimerDef(colorsensor_Tim, RGBsensor_measurement);
  colorsensor_timer_id = osTimerCreate(osTimer(colorsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(colorsensor_timer_id, colorsensor_timer * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void lightsensor_measurement(void){
  osMutexWait(i2cMutexId, osWaitForever);
  int clear_light = getClearData();
  osMutexRelease(i2cMutexId);
  print_light_intensity(clear_light);
}

void RGBsensor_measurement(void){
  struct RGBvalues RGB = {0,0,0};
  turnOnLED();
  HAL_Delay(250);
  osMutexWait(i2cMutexId, osWaitForever);
  RGB.red = getRedData();
  RGB.green = getGreenData();
  RGB.blue = getBlueData();
  osMutexRelease(i2cMutexId);
  turnOffLED();
  print_color(RGB);
}

void print_light_intensity(int data){
  printINF("Light intensity: %d  lux \r\n", data);
}
void print_color(struct RGBvalues values){
  printINF("color red: %d , color green: %d, color blue: %d \r\n", values.red, values.green, values.blue);
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    osDelay(1);
  }
}