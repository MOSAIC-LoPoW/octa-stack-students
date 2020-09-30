#include "gpio.h"
#include "rtosincludes.h"

static void (*OCTA_BTN1_Pressed_Callback)(void) = 0;
static void (*OCTA_BTN2_Pressed_Callback)(void) = 0;
static void (*Nucleo_BTN1_Pressed_Callback)(void) = 0;
static void (*AccInt1_Callback)(void) = 0;

void OCTA_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  HAL_GPIO_WritePin(GPIOE, P2_DIO1_Pin|P2_DIO6_Pin|P2_DIO2_Pin|P2_DIO4_Pin 
                          |P2_DIO3_Pin|P3_DIO1_Pin|P3_DIO4_Pin|P3_DIO5_Pin 
                          |P3_DIO2_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOF, P1_DIO3_Pin|P1_DIO5_Pin|P2_DIO5_Pin|P3_DIO3_Pin , GPIO_PIN_SET);

  #if ENABLE_STEPUP
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_SET);
  #else
    HAL_GPIO_WritePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin, GPIO_PIN_RESET);
  #endif

  HAL_GPIO_WritePin(GPIOC, P1_DIO6_Pin|P1_DIO2_Pin|P1_DIO1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(BLE_Reset_GPIO_Port, BLE_Reset_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(P1_DIO4_GPIO_Port, P1_DIO4_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, OCTA_GLED_Pin|P3_DIO6_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, OCTA_RLED_Pin|OCTA_BLED_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = P2_DIO1_Pin|P2_DIO6_Pin|P2_DIO2_Pin|P2_DIO4_Pin 
                          |P2_DIO3_Pin|P3_DIO1_Pin|P3_DIO4_Pin|P3_DIO5_Pin 
                          |P3_DIO2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = P1_DIO3_Pin|P1_DIO5_Pin|P2_DIO5_Pin|OCTA_STEPUP_Pin 
                          |P3_DIO3_Pin|OCTA_LIGHTSENSORLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = P1_DIO6_Pin|P1_DIO2_Pin|P1_DIO1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BLE_Reset_Pin|P1_DIO4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OCTA_GLED_Pin|P3_DIO6_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OCTA_BTN1_Pin|OCTA_BTN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OCTA_RLED_Pin|OCTA_BLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OCTA_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OCTA_FLASH_CS_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OCTA_FLASH_WP_Pin|OCTA_FLASH_HOLD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  HAL_GPIO_WritePin(OCTA_FLASH_CS_Port, OCTA_FLASH_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOE, OCTA_FLASH_WP_Pin|OCTA_FLASH_HOLD_Pin, GPIO_PIN_SET);

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

  P3_DIO1.PORT = P3_DIO1_GPIO_Port;
  P3_DIO1.PIN =  P3_DIO1_Pin;
  P3_DIO2.PORT = P3_DIO2_GPIO_Port;
  P3_DIO2.PIN = P3_DIO2_Pin;
  P3_DIO3.PORT = P3_DIO3_GPIO_Port;
  P3_DIO3.PIN = P3_DIO3_Pin;
  P3_DIO4.PORT = P3_DIO4_GPIO_Port;
  P3_DIO4.PIN = P3_DIO4_Pin;
  P3_DIO5.PORT = P3_DIO5_GPIO_Port;
  P3_DIO5.PIN = P3_DIO5_Pin;
  P3_DIO6.PORT = P3_DIO6_GPIO_Port;
  P3_DIO6.PIN = P3_DIO6_Pin;
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
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure); 
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);    
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOI, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();  
  __HAL_RCC_GPIOG_CLK_DISABLE();  
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
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
    // Nucleo BTN1
    if (gpioPinNumber==B1_Pin) 
    {
        printDBG("Nucleo BTN1 PRESSED\r\n");
        if(Nucleo_BTN1_Pressed_Callback)
          Nucleo_BTN1_Pressed_Callback();
    }
    // Accelerometer INT1 pin
    if (gpioPinNumber == GPIO_PIN_13)
    {
      printDBG("Accelerometer triggered\r\n");
      if(AccInt1_Callback){
        AccInt1_Callback();
      }
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
    case B1_Pin:
      Nucleo_BTN1_Pressed_Callback = callback;
      break;
    case 13:
      AccInt1_Callback = callback;
      break;
    default:
      break;
  }
}