#include "batterylevel-example.h"
#include "stc3115.h"

#define IWDG_INTERVAL                   5         //seconds
#define BATTERYLEVEL_INTERVAL           60        //seconds

STC3115_ConfigData_TypeDef STC3115_ConfigData;
STC3115_BatteryData_TypeDef STC3115_BatteryData;

int main(void)
{
  Initialize_Platform();

  // Battery monitoring
  GasGauge_Initialization(&common_I2C, &STC3115_ConfigData, &STC3115_BatteryData);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, IWDG_INTERVAL * 1000);
  
  osTimerDef(batteryLevel_Tim, batteryLevel_measurement);
  batteryTimId = osTimerCreate(osTimer(batteryLevel_Tim), osTimerPeriodic, NULL);
  osTimerStart(batteryTimId, BATTERYLEVEL_INTERVAL * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void batteryLevel_measurement(void const *argument)
{
  GasGauge_Task(&STC3115_ConfigData, &STC3115_BatteryData);
  printINF("Vbat: %i mV, I=%i mA SoC=%i, C=%i, P=%i A=%i , CC=%d\r\n",
         STC3115_BatteryData.Voltage,
         STC3115_BatteryData.Current,
         STC3115_BatteryData.SOC,
         STC3115_BatteryData.ChargeValue,
         STC3115_BatteryData.Presence,
         STC3115_BatteryData.StatusWord >> 13,
         STC3115_BatteryData.ConvCounter);
}