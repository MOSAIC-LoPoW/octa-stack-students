#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;
osTimerId temp_hum_timer_id;
osTimerId accelerometer_timer_id;
osTimerId magnetometer_timer_id;
osTimerId lightsensor_timer_id;
osTimerId colorsensor_timer_id;
osTimerId battery_sensor_timer_id;
osMutexId i2cMutexId;
struct RGBvalues
{
  /* data */
  int red, green, blue;
};

void StartDefaultTask(void const *argument);
void temp_hum_measurement(void);
void print_temp_hum(void);
void accelerometer_measurement(void);
void magnetometer_measurement(void);
void print_accelerometer(uint16_t data[]);
void print_magnetometer(uint16_t data[]);
void lightsensor_measurement(void);
void RGBsensor_measurement(void);
void BatterySensor_measurement(void);
void print_light_intensity(int data);
void B1_enable_interrupt(void);
void B1_enable_interrupt(void);
void BTN1_Appcallback(void);
void BTN2_Appcallback(void);
void print_color(struct RGBvalues values);