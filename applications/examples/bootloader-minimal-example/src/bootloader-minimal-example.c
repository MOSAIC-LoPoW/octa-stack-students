#include "bootloader-minimal-example.h"

int main(void)
{
  Initialize_Platform();

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, 0);
  osTimerStart(iwdgTimId, 5 * 1000);

  osKernelStart();

  while (1)
  {
  }
}