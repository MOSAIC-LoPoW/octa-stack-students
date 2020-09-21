#include "iwdg.h"
#include "platform.h"

void OCTA_IWDG_Init(void)
{
  /*
    check if bit 17 of USERConfig byte is set (IWDG_STOP = 1), if yes, set to 0
      Bit 17 IWDG_STOP: Independent watchdog counter freeze in Stop mode
          0: Independent watchdog counter is frozen in Stop mode
          1: Independent watchdog counter is running in Stop mode
  */
  FLASH_OBProgramInitTypeDef OBInitStruct;
  HAL_FLASHEx_OBGetConfig(&OBInitStruct);
  
  if(OBInitStruct.USERConfig & (1 << 17))
  {
      OBInitStruct.OptionType = OPTIONBYTE_USER;
      OBInitStruct.WRPArea = OB_WRPAREA_BANK1_AREAA;
      OBInitStruct.WRPStartOffset = 0;
      OBInitStruct.WRPEndOffset = 0;
      OBInitStruct.RDPLevel = OB_RDP_LEVEL_0;
      OBInitStruct.USERType = OB_USER_IWDG_STOP;
      OBInitStruct.USERConfig = OB_IWDG_STOP_FREEZE;
      OBInitStruct.PCROPConfig = OB_PCROP_RDP_NOT_ERASE;
      OBInitStruct.PCROPStartAddr = 0;
      OBInitStruct.PCROPEndAddr = 0;


      /* USER CODE BEGIN IWDG_Init 0 */
      HAL_FLASH_Unlock();
      HAL_FLASH_OB_Unlock();
      HAL_FLASHEx_OBProgram(&OBInitStruct);
      HAL_FLASH_OB_Lock();
      HAL_FLASH_Lock();
      HAL_FLASH_OB_Launch();
  }
  
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
}

void IWDG_feed(void const *argument)
{
  WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
}