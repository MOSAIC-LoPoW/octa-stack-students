#include "assert.h"
#include "platform.h"

void assert_failed(char *FILE, uint32_t line)
{
    printERR("ASSERT in file: %s on line: %d\r\n", FILE, line);
    // If under debug, infinite loop, else reboot
    if((CoreDebug->DHCSR & 0x1) == 0x1) 
    { /* If under debug */
        // STOP SCHEDULER
        vTaskSuspendAll();
        while(1)
        {
            // feed iwdg
            WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
            // FLASH PURPLE
            HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
            HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
            HAL_Delay(200);
            HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
            HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
            HAL_Delay(200);
        }
    }
    else
    {
        NVIC_SystemReset();
    }
}