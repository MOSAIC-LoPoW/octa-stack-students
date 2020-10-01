#include "acc-interrupt-led.h"
#include "LSM303AGRSensor.h"

#define accelerometer_timer    3
#define LSM303AGR_ACC_CTRL_REG2 0x21
#define LSM303AGR_ACC_CTRL_REG3 0x22
#define LSM303AGR_ACC_INT1_THS 0x32
#define LSM303AGR_ACC_INT1_DUR 0x33
#define LSM303AGR_ACC_INT1_CFG 0x30
#define LSM303AGR_ACC_INT1_SRC 0x31
uint8_t data;
int main(void)
{
  Initialize_Platform();

  LSM303AGR_setI2CInterface(&common_I2C);
  LSM303AGR_ACC_reset();
  HAL_Delay(100);
	//LSM303AGR_init();
  
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
  
  // MUTEX
  osMutexDef(txMutex);
  i2cMutexId = osMutexCreate(osMutex(txMutex));

  configureAccInterruptPin();
  enableAccInterrupt();
  GPIO_SetApplicationCallback(*toggleLed, GPIO_PIN_13);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
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

void print_accelerometer(uint16_t data[]){
  printINF("Accelerometer data: X: %d Y: %d Z: %d \r\n", data[0], data[1], data[2]);
}

void enableAccInterrupt(){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);		
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void configureAccInterruptPin(){
    LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, 0xA7, 0); // REG 1
    LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG2, 0x00, 0); // REG 2
    LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG3, 0x40, 0); // REG 3
    LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG4, 0x00, 0); // REG 4
    LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, 0x00, 0); // REG 5
    LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_THS, 0x10, 0); // INT1_THS_A
    LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_DUR, 0x7F, 0); // INT1_DUR_A
    LSM303AGR_writeRegister(LSM303AGR_ACC_INT1_CFG, 0x0A, 0); // INT1_CFG_A
}

void toggleLed(){
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
  resetInterrupt();
}

void resetInterrupt(){
  LSM303AGR_readRegister(LSM303AGR_ACC_INT1_SRC, data, 0);
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    osDelay(1);
  }
}