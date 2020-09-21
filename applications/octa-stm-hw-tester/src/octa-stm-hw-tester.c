#include "octa-stm-hw-tester.h"
#include "sht31.h"
#include "LSM303AGRSensor.h"
#include "TCS3472.h"
#include "bq35100.h"
#include "octa-flash.h"

float SHTData[2];
uint8_t buffer[1] = {0};
char cmd[30] = {0};
char toParse[30] = {0};
uint8_t cmdindex = 0;
uint8_t cmdToParse = 0;

int main(void)
{
  Initialize_Platform();

  printINF("---------------------------------------\r\n");
  printINF("|octa-stm hardware testing application|\r\n");
  printINF("---------------------------------------\r\n\r\n");

  list_commands();

  // Enable button interrupt for test mode
  B1_enable_interrupt();
  GPIO_SetApplicationCallback(&BTN1_Appcallback, (uint16_t)OCTA_BTN1_Pin);

  // Enable button interrupt for flash offloading
  B2_enable_interrupt();
  GPIO_SetApplicationCallback(&BTN2_Appcallback, (uint16_t)OCTA_BTN2_Pin);

  UART_USB_SetRxCallback(&USB_UART_CallBack);
  HAL_UART_Receive_IT(&USB_UART, buffer, 1);

  while (1)
  {
    IWDG_feed(NULL);
    if(cmdToParse)
    {
      parse_cmd();
      cmdToParse = 0;
    }
  }
}

void USB_UART_CallBack(void)
{
  if(!cmdindex)
  {
    if(buffer[0] == 't')
    {
      memset(cmd, 0, 30);
      cmd[cmdindex] = buffer[0];
      cmdindex++;
    }
  }
  else
  {
    if(buffer[0] != '\r')
    {
      cmd[cmdindex] = buffer[0];
      cmdindex++;
    }
    else
    {
      printINF("cmd found: %s\r\n", cmd);
      cmdToParse = 1;
      cmdindex = 0;
    }
  }
  HAL_UART_Receive_IT(&USB_UART, buffer, 1);    
}

void parse_cmd(void)
{
    if(strstr(cmd, "testList")!=0)
    { 
      list_commands();
    }
    else if(strstr(cmd, "testRLED")!=0)
    { 
      printINF("RLED toggle for 3s\r\n");
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
      HAL_Delay(3000);
      HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    }
    else if(strstr(cmd, "testGLED")!=0)
    { 
      printINF("GLED toggle for 3s\r\n");
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
      HAL_Delay(3000);
      HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    }
    else if(strstr(cmd, "testBLED")!=0)
    { 
      printINF("BLED toggle for 3s\r\n");
      HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
      HAL_Delay(3000);
      HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    }
    else if(strstr(cmd, "testSensorVCCSwitch")!=0)
    { 
      printINF("Set sensors VCC Switch to HIGH\r\n");
      HAL_GPIO_WritePin(OCTA_VCC_SENSORS_Port, OCTA_VCC_SENSORS_Pin, 1);
    }
    else if(strstr(cmd, "testTempHumSensor")!=0)
    {
      testTempHum();
    }
    else if(strstr(cmd, "testLSMSensor")!=0)
    {
      testLSM();
    }
    else if(strstr(cmd, "testLightSensor")!=0)
    {
      testLight();
    }
    else if(strstr(cmd, "testBatteryManagement")!=0)
    {
      testBatteryManagement();
    }
    else if(strstr(cmd, "testFlashVCCSwitch")!=0)
    { 
      printINF("Set flash VCC Switch to HIGH\r\n");
      HAL_GPIO_WritePin(OCTA_VCC_FLASH_Port, OCTA_VCC_FLASH_Pin, 1);
    }
    else if(strstr(cmd, "testFlashRW")!=0)
    { 
      printINF("OCTA Flash RW test\r\n");
      OCTA_FLASH_Initialize(&FLASH_SPI);
      uint8_t result = OCTA_FLASH_TestRW();
      if(result)
        printINF("Flash RW test OK\r\n");
      else
        printINF("Flash RW test NOK\r\n");
    }
    else if(strstr(cmd, "testNBIOTVCCSwitch")!=0)
    { 
      printINF("Set NB-IoT VCC Switch to HIGH\r\n");
      HAL_GPIO_WritePin(OCTA_VCC_NBIOT_Port, OCTA_VCC_NBIOT_Pin, 1);
    }
    else if(strstr(cmd, "testNBIOTUARTSwitch")!=0)
    { 
      printINF("Set NB-IoT UART Switch to HIGH\r\n");
      HAL_GPIO_WritePin(OCTA_NBIOT_UARTSWITCH_Port, OCTA_NBIOT_UARTSWITCH_Pin, 1);
    }
    else if(strstr(cmd, "testNBIOTUARTCheck")!=0)
    { 
      printINF("Testing NB-IoT UART communication\r\n");
      NBIOT_UART_Init(115200); 
      const char* cmd = "AT+CGMI\r\n";
      uint8_t buff[25];
      memset(buff, 0, 25);
      HAL_UART_Transmit(&NBIOT_UART, (uint8_t *)cmd, strlen(cmd), 200);
      HAL_UART_AbortReceive(&NBIOT_UART);
      HAL_StatusTypeDef status = HAL_UART_Receive(&NBIOT_UART, (uint8_t *)buff, 25, 1000);
      printINF("NB-IoT Chip INFO: %s, statusrx %d\r\n", buff, status);
    }
    else if(strstr(cmd, "test5VStepup")!=0)
    { 
      printINF("Set 5V Stepup Pin to HIGH\r\n");
      HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, 1);
    }
    else if(strstr(cmd, "testRS232")!=0)
    { 
      printINF("Testing RS232 <-> UART communication\r\n");
      RS232_UART_Init(9600); 
      uint8_t rs232buff[50];
      memset(rs232buff, 0, 50);
      HAL_UART_AbortReceive(&RS232_UART);
      HAL_StatusTypeDef rs232status = HAL_UART_Receive(&RS232_UART, (uint8_t *)rs232buff, 50, 5000);
      printINF("RS232 rx data: %s, statusrx %d\r\n", rs232buff, rs232status);
    }
}

void testTempHum(void)
{
  printINF("reading out SHT31 sensor 3 times\r\n");
  //SHT
  setI2CInterface_SHT31(&common_I2C);
  SHT31_begin();
  HAL_Delay(500);
  temp_hum_measurement();
  HAL_Delay(500);
  temp_hum_measurement();
  HAL_Delay(500);
  temp_hum_measurement();
}

void testLSM(void)
{
  //ACC-MAG
  LSM303AGR_setI2CInterface(&common_I2C);
  LSM303AGR_ACC_reset();
	LSM303AGR_MAG_reset();
  HAL_Delay(100);
	LSM303AGR_init();
  HAL_Delay(500);
  accelerometer_measurement();
  magnetometer_measurement();
  HAL_Delay(500);
  accelerometer_measurement();
  magnetometer_measurement();
  HAL_Delay(500);
  accelerometer_measurement();
  magnetometer_measurement();
}

void testLight(void)
{
  TC3472_init(&common_I2C);
  HAL_Delay(500);
  lightsensor_measurement();
  RGBsensor_measurement();
  HAL_Delay(500);
  lightsensor_measurement();
  RGBsensor_measurement();
  HAL_Delay(500);
  lightsensor_measurement();
  RGBsensor_measurement();
}

void testBatteryManagement(void)
{
  //BATTERY GAUGE
  bq35100_init(&common_I2C);
  bq35100_data_struct bq35100_data;
  bq35100_read_acc_data(&bq35100_data);
  printINF("bq35100 temp %d, volts %d, current %d, acc capacity %d\r\n", bq35100_data.temp, bq35100_data.volt, bq35100_data.current, bq35100_data.acc_capacity);
}

void testRS232(void)
{
  
}

void list_commands(void)
{
  printINF("Type one of the following commands followed by a carriage return:\r\n");
  printINF("- testRLED\r\n");
  printINF("- testGLED\r\n");
  printINF("- testBLED\r\n");
  printINF("- testSensorVCCSwitch\r\n");
  printINF("- testTempHumSensor\r\n");
  printINF("- testLSMSensor\r\n");
  printINF("- testLightSensor\r\n");
  printINF("- testBatteryManagement\r\n");
  printINF("- testFlashVCCSwitch\r\n");
  printINF("- testFlashRW\r\n");
  printINF("- test5VStepup\r\n");
  printINF("- testRS232\r\n");
  printINF("- testNBIOTVCCSwitch\r\n");
  printINF("- testNBIOTUARTSwitch\r\n");
  printINF("- testNBIOTUARTCheck\r\n");
  printINF("- testNBIOTRF\r\n");

  printINF("Type \"testList\" to get the list of commands again\r\n");
  printINF("To test the buttons, press them\r\n");
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

void accelerometer_measurement(void){
	uint16_t accDataRaw[3];
  LSM303AGR_ACC_readAccelerationRawData(accDataRaw);
  print_accelerometer(accDataRaw);
}

void magnetometer_measurement(void){
  uint16_t magDataRaw[3];
  LSM303AGR_MAG_readMagneticRawData(magDataRaw);
  print_magnetometer(magDataRaw);
}

void print_accelerometer(uint16_t data[]){
  printINF("Accelerometer data: X: %d Y: %d Z: %d \r\n", data[0], data[1], data[2]);
}
void print_magnetometer(uint16_t data[]){
  printINF("Magnetometer data: X: %d Y: %d Z: %d \r\n", data[0], data[1], data[2]);
}

void lightsensor_measurement(void){
  int clear_light = getClearData();
  print_light_intensity(clear_light);
}

void RGBsensor_measurement(void){
  struct RGBvalues RGB = {0,0,0};
  turnOnLED();
  HAL_Delay(250);
  RGB.red = getRedData();
  RGB.green = getGreenData();
  RGB.blue = getBlueData();
  turnOffLED();
  print_color(RGB);
}

void print_light_intensity(int data)
{
  printINF("Light intensity: %d  lux \r\n", data);
}

void print_color(struct RGBvalues values)
{
  printINF("color red: %d , color green: %d, color blue: %d \r\n", values.red, values.green, values.blue);
}