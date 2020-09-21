#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;
osTimerId accelerometer_timer_id;
osTimerId magnetometer_timer_id;
osMutexId i2cMutexId;

void StartDefaultTask(void const *argument);

void accelerometer_measurement(void);
void magnetometer_measurement(void);
void print_accelerometer(uint16_t data[]);
void print_magnetometer(uint16_t data[]);
