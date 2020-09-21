#include "error.h"
#include "platform.h"

void Error_Handler(void)
{
    printERR("Error Handler\r\n");
    // If under debug, infinite loop, else reboot
    if((CoreDebug->DHCSR & 0x1) == 0x1) 
    { /* If under debug */
        // STOP SCHEDULER
        vTaskSuspendAll();
        while(1)
        {
            // feed iwdg
            WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
            // FLASH YELLOW
            HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
            HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
            HAL_Delay(200);
            HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
            HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
            HAL_Delay(200);
        }
    }
    else
    {
        NVIC_SystemReset();
    }
}