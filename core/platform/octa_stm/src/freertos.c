#include "FreeRTOS.h"
#include "task.h"
#include "rtosincludes.h"
#include "platform.h"

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECKFORSTACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
  printERR("*****FreeRTOS stack overflow*****\r\n");
  asm("bkpt 255");
}

void RTOS_Print_Stats_Thread(void const *argument)
{
  for (;;)
  {
    RTOS_print_stats();
    osThreadSuspend(NULL);
  }
}


void RTOS_Send_Notification(osThreadId threadid)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    TaskHandle_t xTaskToNotify = threadid;
    /* At this point xTaskToNotify should not be NULL as a transmission was
        in progress. */
    configASSERT(xTaskToNotify != NULL);

    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);

    /* There are no transmissions in progress, so no tasks to notify. */
    xTaskToNotify = NULL;

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
        should be performed to ensure the interrupt returns directly to the highest
        priority task.  The macro used for this purpose is dependent on the port in
        use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

#if USE_RTOS_SCHEDULER && DEBUG
    char *pcConvertThreadState(eTaskState state) {
        switch (state) {
            case eReady:
                return "READY";
            case eBlocked:
                return "BLOCKED";
            case eSuspended:
                return "SUSPENDED";
            case eDeleted:
                return "DELETED";

            default: /* Should not get here, but it is included
                        to prevent static checking errors. */
                return "UNKNOWN";
        }
    }
    void RTOS_print_stats(void)
    {
        TaskStatus_t *pxTaskStatusArray = NULL;
        uint32_t ulTotalRuntime;
      
      /* Allocate an array index for each task. */
      pxTaskStatusArray = pvPortMalloc( uxTaskGetNumberOfTasks() * sizeof( TaskStatus_t ) );
      

      if(pxTaskStatusArray != NULL ) {
        /* Generate the (binary) data. */
        uxTaskGetSystemState( pxTaskStatusArray, uxTaskGetNumberOfTasks(), &ulTotalRuntime );

        printDBG("         LIST OF RUNNING THREADS         \r\n-----------------------------------------\r\n");

        for(uint16_t i = 0; i < uxTaskGetNumberOfTasks(); i++ ) {
          printDBG("Thread: %s\r\n", pxTaskStatusArray[i].pcTaskName);

          printDBG("Thread ID: %lu\r\n", pxTaskStatusArray[i].xTaskNumber);

          printDBG("\tStatus: %s\r\n", pcConvertThreadState(pxTaskStatusArray[i].eCurrentState));

          printDBG("\tStack watermark number: %d\r\n", pxTaskStatusArray[i].usStackHighWaterMark);

          printDBG("\tPriority: %lu\r\n", pxTaskStatusArray[i].uxCurrentPriority);
        }
        vPortFree(pxTaskStatusArray);
        }
    }
#endif