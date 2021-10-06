#include "tickless-octa-stm-example.h"
#include "NORDIC9160.h"

int main(void)
{
  Initialize_Platform();

  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  // set application RTC wakeup callback
  RTC_SetApplicationWakeupCallback(&RTC_Wakeup);

  // set application specific Reinit function, called after platform deinit
  set_Application_ReInit_Function(&Application_ReInit);
  // set application specific Deinit function, called after platform deinit
  set_Application_DeInit_Function(&Application_DeInit);

  HAL_Delay(3000);

  //HAL_SuspendTick();

  osKernelStart();

  while (1)
  {
  }
}

void StartDefaultTask(void const *argument)
{
  for (;;)
  {
    IWDG_feed(NULL);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    osDelay(1000);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    IWDG_feed(NULL);

    printDBG("Setting RTC wakeup for 10s -> will enter stop mode for as there is nothing to do\r\n");
    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 10, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
    {
      Error_Handler();
    }
    osThreadSuspend(NULL);
  }
}

void RTC_Wakeup(void)
{
  osThreadResume(defaultTaskHandle);
}

// this is for demo purposes, should reinit app specific peripherals
void Application_ReInit(void)
{
  printINF("Application Reinit Function\r\n");
}

// this is for demo purposes, should Deinit app specific peripherals
void Application_DeInit(void)
{
  //does nothing for now
}
