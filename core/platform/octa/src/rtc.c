#include "rtc.h"
#include "platform.h"
#include "time.h"

static void (*RTC_Wakeup_ApplicationCallback)(void) = 0;

void RTC_Init(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_IT();
}

/**
   * Description: Unix timestamp parsing and setting local time
    * Parameter: UnixNum Unix timestamp
    * Return: NULL
    * Others:
  */
void RTC_Set_Time_From_Unix(uint32_t UnixNum)
{
  struct tm *stmU;

  RTC_DateTypeDef sdate;
  RTC_TimeTypeDef stime;

  time_t Count;

  Count = UnixNum;
          
  stmU = localtime(&Count);
  sdate.Year = stmU->tm_year - 100;
  sdate.Month = stmU->tm_mon + 1;
  sdate.Date = stmU->tm_mday;
  sdate.WeekDay = stmU->tm_wday + 1;
  stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  stime.SubSeconds = 0;
  stime.SecondFraction = 0;
  stime.StoreOperation = RTC_STOREOPERATION_SET;
  stime.Hours = stmU->tm_hour;
  stime.Minutes = stmU->tm_min;
  stime.Seconds = stmU->tm_sec;

  if(stime.Hours > 24 || stime.Minutes > 60 || stime.Seconds > 60 || sdate.Year > 4000 || sdate.Month > 12 || sdate.Date > 31)
  {
    printERR("Invalid utc timestamp. not updating RTC\r\n");
    return;
  }

  HAL_RTC_SetDate(&hrtc,&sdate,RTC_FORMAT_BIN);
  HAL_RTC_SetTime(&hrtc,&stime,RTC_FORMAT_BIN);
  
  printDBG("RTC datetime updated\r\n");
}

/*
* Description: Local time to generate Unix timestamps
* Parameter:
* Return: 
* Others:
*/
uint32_t RTC_Time_To_Unix()
{
  char buf[50];
          
  struct tm stmT;
             
  RTC_DateTypeDef sdate;
  RTC_TimeTypeDef stime;
  
  HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
      
  stmT.tm_year=sdate.Year+100;
  stmT.tm_mon=sdate.Month-1;  
  stmT.tm_mday=sdate.Date;  
  stmT.tm_hour=stime.Hours;  
  stmT.tm_min=stime.Minutes;  
  stmT.tm_sec=stime.Seconds;  
          
  uint32_t unixtimestamp = (uint32_t)mktime(&stmT);
  printDBG("UTC timestamp: %d \r\n",unixtimestamp);
  return unixtimestamp;
}

void RTC_Print_DateTime()
{
  RTC_DateTypeDef sdate;
  RTC_TimeTypeDef stime;
  HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
  printDBG("Date (YYYY/MM/DD): %02d/%02d/%02d\r\n",2000 + sdate.Year, sdate.Month, sdate.Date);
  printDBG("Time (HH:MM:SS): %02d:%02d:%02d\r\n",stime.Hours, stime.Minutes, stime.Seconds); 
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrtc);
  if(RTC_Wakeup_ApplicationCallback)
    RTC_Wakeup_ApplicationCallback();
}

void RTC_SetApplicationWakeupCallback(void (*callback))
{
  RTC_Wakeup_ApplicationCallback = callback;
}