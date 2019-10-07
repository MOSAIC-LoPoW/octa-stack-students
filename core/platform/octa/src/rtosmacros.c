#include "rtosmacros.h"

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