#include "gpio.h"
#include "rtosincludes.h"

static void (*OCTA_BTN1_Pressed_Callback)(void) = 0;
static void (*OCTA_BTN2_Pressed_Callback)(void) = 0;

void OCTA_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  HAL_PWREx_EnableVddIO2();

  //SWITCHES
  HAL_GPIO_WritePin(OCTA_VCC_NBIOT_Port, OCTA_VCC_NBIOT_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OCTA_VCC_SENSORS_Port, OCTA_VCC_SENSORS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OCTA_VCC_FLASH_Port, OCTA_VCC_FLASH_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OCTA_NBIOT_UARTSWITCH_Port, OCTA_NBIOT_UARTSWITCH_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(P1_DIO1_GPIO_Port, P1_DIO1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P1_DIO2_GPIO_Port, P1_DIO2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P1_DIO3_GPIO_Port, P1_DIO3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P1_DIO4_GPIO_Port, P1_DIO4_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P1_DIO5_GPIO_Port, P1_DIO5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P1_DIO6_GPIO_Port, P1_DIO6_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(P2_DIO1_GPIO_Port, P2_DIO1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P2_DIO2_GPIO_Port, P2_DIO2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P2_DIO3_GPIO_Port, P2_DIO3_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P2_DIO4_GPIO_Port, P2_DIO4_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P2_DIO5_GPIO_Port, P2_DIO5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(P2_DIO6_GPIO_Port, P2_DIO6_Pin, GPIO_PIN_RESET);

  #if ENABLE_STEPUP
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_SET);
  #else
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_RESET);
  #endif

  HAL_GPIO_WritePin(GPIOE, OCTA_GLED_Pin|OCTA_RLED_Pin|OCTA_BLED_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(OCTA_FLASH_WP_Port, OCTA_FLASH_WP_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OCTA_FLASH_HOLD_Port, OCTA_FLASH_HOLD_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(OCTA_FLASH_CS_Port, OCTA_FLASH_CS_Pin, GPIO_PIN_SET);

  //GPIO A
  GPIO_InitStruct.Pin = OCTA_VCC_SENSORS_Pin|OCTA_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //GPIO B
  GPIO_InitStruct.Pin = P1_DIO3_Pin|P1_DIO4_Pin|P1_DIO5_Pin|OCTA_STEPUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  //GPIO C
  GPIO_InitStruct.Pin = P1_DIO1_Pin|P1_DIO2_Pin|P2_DIO5_Pin|P2_DIO6_Pin|OCTA_FLASH_HOLD_Pin|OCTA_NBIOT_Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  //GPIO D
  GPIO_InitStruct.Pin = P2_DIO1_Pin|P2_DIO2_Pin|P2_DIO3_Pin|P2_DIO4_Pin|OCTA_FLASH_WP_Pin|OCTA_VCC_FLASH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  //GPIO D
  GPIO_InitStruct.Pin = OCTA_GAUGE_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OCTA_GAUGE_ENABLE_Port, &GPIO_InitStruct);

  //GPIO E
  GPIO_InitStruct.Pin = P1_DIO6_Pin|OCTA_GLED_Pin|OCTA_RLED_Pin|OCTA_BLED_Pin|OCTA_VCC_NBIOT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  //OCTA_BUTTONS
  GPIO_InitStruct.Pin = OCTA_BTN1_Pin|OCTA_BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  P1_DIO1.PORT = P1_DIO1_GPIO_Port;
  P1_DIO1.PIN =  P1_DIO1_Pin;
  P1_DIO2.PORT = P1_DIO2_GPIO_Port;
  P1_DIO2.PIN = P1_DIO2_Pin;
  P1_DIO3.PORT = P1_DIO3_GPIO_Port;
  P1_DIO3.PIN = P1_DIO3_Pin;
  P1_DIO4.PORT = P1_DIO4_GPIO_Port;
  P1_DIO4.PIN = P1_DIO4_Pin;
  P1_DIO5.PORT = P1_DIO5_GPIO_Port;
  P1_DIO5.PIN = P1_DIO5_Pin;
  P1_DIO6.PORT = P1_DIO6_GPIO_Port;
  P1_DIO6.PIN = P1_DIO6_Pin;

  P2_DIO1.PORT = P2_DIO1_GPIO_Port;
  P2_DIO1.PIN =  P2_DIO1_Pin;
  P2_DIO2.PORT = P2_DIO2_GPIO_Port;
  P2_DIO2.PIN = P2_DIO2_Pin;
  P2_DIO3.PORT = P2_DIO3_GPIO_Port;
  P2_DIO3.PIN = P2_DIO3_Pin;
  P2_DIO4.PORT = P2_DIO4_GPIO_Port;
  P2_DIO4.PIN = P2_DIO4_Pin;
  P2_DIO5.PORT = P2_DIO5_GPIO_Port;
  P2_DIO5.PIN = P2_DIO5_Pin;
  P2_DIO6.PORT = P2_DIO6_GPIO_Port;
  P2_DIO6.PIN = P2_DIO6_Pin;

  NBIOT_Reset.PORT = OCTA_NBIOT_Reset_Port;
  NBIOT_Reset.PIN = OCTA_NBIOT_Reset_Pin;
}

void OCTA_GPIO_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
  /* Note: Debug using ST-Link is not possible during the execution of this   */
  /*       example because communication between ST-link and the device       */
  /*       under test is done through UART. All GPIO pins are disabled (set   */
  /*       to analog input mode) including  UART I/O pins.           */
  GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure); 
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
}

void HAL_GPIO_EXTI_Callback(uint16_t gpioPinNumber) 
{
    // OCTA B1
    if (gpioPinNumber==OCTA_BTN1_Pin) 
    {
        printDBG("OCTA BTN1 PRESSED\r\n");
        if(OCTA_BTN1_Pressed_Callback)
          OCTA_BTN1_Pressed_Callback();
    }
    // OCTA B2
    if (gpioPinNumber==OCTA_BTN2_Pin) 
    {
        printDBG("OCTA BTN2 PRESSED\r\n");
        #if DEBUG && PRINT_RTOS_STATS
          //RESUME rtos print handle, which will print the stats once
          if(osKernelRunning())
            osThreadResume(rtosprintHandle);
          else
            printDBG("Kernel not running\r\n");
        #endif
        if(OCTA_BTN2_Pressed_Callback)
          OCTA_BTN2_Pressed_Callback();
    }
}

void GPIO_SetApplicationCallback(void (*callback), uint16_t pinNumber)
{
  switch(pinNumber)
  {
    case OCTA_BTN1_Pin:
      OCTA_BTN1_Pressed_Callback = callback;
      break;
    case OCTA_BTN2_Pin:
      OCTA_BTN2_Pressed_Callback = callback;
      break;
    default:
      break;
  }
}