#include "gps-firefly-example.h"
#include "fireflyx1.h"

float locationArray[3];
uint8_t locationArrayHex[32];
uint8_t hasGPSFix = 0;
gps_position_dd_t currentLocation={0,0,0};

int main(void)
{
  Initialize_Platform();

  // Firefly-GPS
  Firefly_Initialize();
  memset(locationArrayHex, 1, 32); // init location array to 0

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(GPSTim, GPS_Read);
  gpsTimId = osTimerCreate(osTimer(GPSTim), osTimerPeriodic, NULL);
  osTimerStart(gpsTimId, 10 * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void GPS_Read(void const *argument)
{
    uint8_t response[255];
    uint8_t quality = Firefly_receive(response);
    if(quality==1)
    {   
        printINF("GPS FIX\r\n");
        setGPSCoordinates();
    }
    else
    {
        hasGPSFix=0;
        printINF("NO GPS FIX\r\n");
    }
    
}

void setGPSCoordinates(void)
{
	hasGPSFix=1;
	currentLocation=gps_get_position_dd();
	locationArray[0]=currentLocation.latitude;
	locationArray[1]=currentLocation.longitude;
	locationArray[2]=currentLocation.hdop;
    printINF("Location lat: %f, long: %f, hdop: %f \r\n", locationArray[0], locationArray[1], locationArray[2]);
}