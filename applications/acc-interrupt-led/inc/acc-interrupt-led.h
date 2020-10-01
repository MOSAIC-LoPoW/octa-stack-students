#include "platform.h"

osThreadId defaultTaskHandle;
osTimerId iwdgTimId;
osTimerId accelerometer_timer_id;
osMutexId i2cMutexId;

void StartDefaultTask(void const *argument);

void accelerometer_measurement(void);
void print_accelerometer(uint16_t data[]);
void enableAccInterrupt();
void configureAccInterruptPin();
void toggleLed();
void resetInterrupt();

