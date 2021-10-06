#include "acc-mag-sensor.h"
#include "LSM303AGRSensor.h"

#define accelerometer_timer    3
#define magnetometer_timer    3

int main(void)
{
  Initialize_Platform();

  LSM303AGR_setI2CInterface(&common_I2C);
  LSM303AGR_ACC_reset();
	LSM303AGR_MAG_reset();
  HAL_Delay(100);
	LSM303AGR_init();
  
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(accsensor_Tim, accelerometer_measurement);
  accelerometer_timer_id = osTimerCreate(osTimer(accsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(accelerometer_timer_id, accelerometer_timer * 1000);

  osTimerDef(magsensor_Tim, magnetometer_measurement);
  magnetometer_timer_id = osTimerCreate(osTimer(magsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(magnetometer_timer_id, magnetometer_timer * 1000);
  
  // MUTEX
  osMutexDef(txMutex);
  i2cMutexId = osMutexCreate(osMutex(txMutex));

  osKernelStart();

  while (1)
  {
  }
}

void accelerometer_measurement(void){
	uint16_t accDataRaw[3];
  osMutexWait(i2cMutexId, osWaitForever);
  LSM303AGR_ACC_readAccelerationRawData(accDataRaw);
  osMutexRelease(i2cMutexId);
  print_accelerometer(accDataRaw);
}

void magnetometer_measurement(void){
  uint16_t magDataRaw[3];
  osMutexWait(i2cMutexId, osWaitForever);
  LSM303AGR_MAG_readMagneticRawData(magDataRaw);
  osMutexRelease(i2cMutexId);
  print_magnetometer(magDataRaw);
}

void print_accelerometer(uint16_t data[]){
  printINF("Accelerometer data: X: %d Y: %d Z: %d \r\n", data[0], data[1], data[2]);
}
void print_magnetometer(uint16_t data[]){
  printINF("Magnetometer data: X: %d Y: %d Z: %d \r\n", data[0], data[1], data[2]);
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    osDelay(1);
  }
}