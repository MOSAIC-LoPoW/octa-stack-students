#include "octa-stm-example.h"
#include "sht31.h"
#include "LSM303AGRSensor.h"
#include "TCS3472.h"
#include "octa-flash.h"
#include "bq35100.h"

#define sensor_timer    5

float SHTData[2];
bq35100_data_struct bq35100_data;

int main(void)
{
  Initialize_Platform();

  //enable sensor VCC
  HAL_GPIO_WritePin(OCTA_VCC_SENSORS_Port, OCTA_VCC_SENSORS_Pin, GPIO_PIN_SET);

  //SHT
  setI2CInterface_SHT31(&common_I2C);
  SHT31_begin();

  //ACC-MAG
  LSM303AGR_setI2CInterface(&common_I2C);
  LSM303AGR_ACC_reset();
	LSM303AGR_MAG_reset();
  HAL_Delay(100);
	LSM303AGR_init();

  //lightsensor
  TC3472_init(&common_I2C);

  //BATTERY GAUGE
  bq35100_init(&common_I2C);

  //NB-IoT
  // HAL_GPIO_WritePin(OCTA_VCC_NBIOT_Port, OCTA_VCC_NBIOT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OCTA_NBIOT_UARTSWITCH_Port, OCTA_NBIOT_UARTSWITCH_Pin, GPIO_PIN_SET);
  NBIOT_UART_Init(115200); 

  //RS232
  HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_SET);
  RS232_UART_Init(9600);
  uint8_t buff[20];
  memset(buff, 0, 20);
  HAL_Delay(3000);
  IWDG_feed(NULL);
  HAL_UART_AbortReceive(&RS232_UART);
  HAL_StatusTypeDef statusrx = HAL_UART_Receive(&RS232_UART, (uint8_t *)buff, 20, 2500);
  printINF("RS232: %s, statusrx %d \r\n", buff, statusrx);

  //Flash
  HAL_GPIO_WritePin(OCTA_VCC_FLASH_Port, OCTA_VCC_FLASH_Pin, GPIO_PIN_SET);
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
  uint8_t buffer[256];
  memset(buffer, 20, 256);
  OCTA_FLASH_write(buffer, 256);
  //read to check
  uint8_t rx[256];
  OCTA_FLASH_read(rx,256);
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
  OCTA_FLASH_read(rx,256);
  printDBG("sector 20: \r\n");
  for(uint16_t i = 0; i<256; i++)
      printf("%d ", rx[i]); 
  printf("\r\n");

  if(RWsuccess)
      printINF("octa-flash RW test OK\r\n");
  else
      printINF("octa-flash RW test NOK\r\n");

  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(temp_hum_Tim, temp_hum_measurement);
  temp_hum_timer_id = osTimerCreate(osTimer(temp_hum_Tim), osTimerPeriodic, NULL);
  osTimerStart(temp_hum_timer_id, sensor_timer * 1000);

  osTimerDef(accsensor_Tim, accelerometer_measurement);
  accelerometer_timer_id = osTimerCreate(osTimer(accsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(accelerometer_timer_id, sensor_timer * 1000);

  osTimerDef(magsensor_Tim, magnetometer_measurement);
  magnetometer_timer_id = osTimerCreate(osTimer(magsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(magnetometer_timer_id, sensor_timer * 1000);

  osTimerDef(lightsensor_Tim, lightsensor_measurement);
  lightsensor_timer_id = osTimerCreate(osTimer(lightsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(lightsensor_timer_id, sensor_timer * 1000);

  osTimerDef(colorsensor_Tim, RGBsensor_measurement);
  colorsensor_timer_id = osTimerCreate(osTimer(colorsensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(colorsensor_timer_id, sensor_timer * 1000);

  osTimerDef(battery_sensor_Tim, BatterySensor_measurement);
  battery_sensor_timer_id = osTimerCreate(osTimer(battery_sensor_Tim), osTimerPeriodic, NULL);
  osTimerStart(battery_sensor_timer_id, sensor_timer*3 * 1000);

  // MUTEX
  osMutexDef(txMutex);
  i2cMutexId = osMutexCreate(osMutex(txMutex));

  // Enable button interrupt for test mode
  B1_enable_interrupt();
  GPIO_SetApplicationCallback(&BTN1_Appcallback, (uint16_t)OCTA_BTN1_Pin);

  // Enable button interrupt for flash offloading
  B2_enable_interrupt();
  GPIO_SetApplicationCallback(&BTN2_Appcallback, (uint16_t)OCTA_BTN2_Pin);

  osKernelStart();

  while (1)
  {
  }
}

void B1_enable_interrupt()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OCTA_BTN1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OCTA_BTN1_GPIO_Port, &GPIO_InitStruct);

    //enable the interrupt
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);		
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void B2_enable_interrupt()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OCTA_BTN2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OCTA_BTN2_GPIO_Port, &GPIO_InitStruct);

    //enable the interrupt
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);		
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void BTN1_Appcallback(void)
{
    printINF("BTN1 pressed\r\n");
}

void BTN2_Appcallback(void)
{
    printINF("BTN2 pressed\r\n");
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    const char* cmd = "AT+CGMI\r\n";
    uint8_t buff[25];
    memset(buff, 0, 25);
    HAL_StatusTypeDef statustx,statusrx;
    statustx = HAL_UART_Transmit(&NBIOT_UART, (uint8_t *)cmd, strlen(cmd), 300);
    HAL_UART_AbortReceive(&NBIOT_UART);
    statusrx = HAL_UART_Receive(&NBIOT_UART, (uint8_t *)buff, 25, 1000);
    printINF("NORDIC CGMI: %s, statustx %d, statusrx %d\r\n", buff, statustx, statusrx);

    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    osDelay(3000);
  }
}

void temp_hum_measurement(void)
{
  osMutexWait(i2cMutexId, osWaitForever);
  SHT31_get_temp_hum(SHTData);
  osMutexRelease(i2cMutexId);
  print_temp_hum();
}

void print_temp_hum(void)
{
  printINF("Temperature: %.2f degC \r\n", SHTData[0]);
  printINF("Humidity: %.2f %% \r\n", SHTData[1]);
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

void BatterySensor_measurement(void)
{
  bq35100_read_acc_data(&bq35100_data);
  printINF("bq35100 temp %d, volts %d, current %d, acc capacity %d\r\n", bq35100_data.temp, bq35100_data.volt, bq35100_data.current, bq35100_data.acc_capacity);
}

void print_light_intensity(int data)
{
  printINF("Light intensity: %d  lux \r\n", data);
}

void print_color(struct RGBvalues values)
{
  printINF("color red: %d , color green: %d, color blue: %d \r\n", values.red, values.green, values.blue);
}