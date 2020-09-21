#include "platform.h"

osTimerId iwdgTimId;
osTimerId gpsTimId;

void setGPSCoordinates();
void GPS_Read(void const *argument);