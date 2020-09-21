#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;
osTimerId temp_hum_timer_id;
osTimerId accelerometer_timer_id;
osTimerId magnetometer_timer_id;
osTimerId lightsensor_timer_id;
osTimerId colorsensor_timer_id;
osMutexId i2cMutexId;
struct RGBvalues
{
  /* data */
  int red, green, blue;
};

void parse_cmd(void);
void list_commands(void);
void testTempHum(void);
void testLight(void);
void testLSM(void);
void testBatteryManagement(void);
void testRS232(void);
void temp_hum_measurement(void);
void print_temp_hum(void);
void accelerometer_measurement(void);
void magnetometer_measurement(void);
void print_accelerometer(uint16_t data[]);
void print_magnetometer(uint16_t data[]);
void lightsensor_measurement(void);
void RGBsensor_measurement(void);
void print_light_intensity(int data);
void B1_enable_interrupt(void);
void B1_enable_interrupt(void);
void BTN1_Appcallback(void);
void BTN2_Appcallback(void);
void USB_UART_CallBack(void);
void print_color(struct RGBvalues values);