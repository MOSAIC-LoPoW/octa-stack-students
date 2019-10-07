/**
 * \file        nova_PM_sensor.c
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
  *
 * \details     Driver for Nova PM sensor.
 *
 * \author      Dragan
 * \date        09-2019
 */
#include "platform.h"
#include "nova_PM_sensor.h"

struct OCTA_header nova_PM_Header;
UART_HandleTypeDef *uarthandle;
char tmpbuf[10];
/************************************************************************/
/* Public Methods                                                       */
/************************************************************************/

uint8_t nova_PM_Initialize(void)
{
    printf("***Initializing Nova PM sensor driver***\r\n");

    #ifndef NOVA_PM_CONNECTOR
        printf("No NOVA_PM_CONNECTOR provided in Makefile\r\n");
        return 0;
    #else
        nova_PM_Header = platform_getHeader((uint8_t)NOVA_PM_CONNECTOR);
        if(!nova_PM_Header.active)
        {
            printf("Invalid NOVA_PM_CONNECTOR provided in Makefile\r\n");
            return 0;  
        }
        else   
            printf("Nova PM sensor on P%d, initializing UART\r\n", (uint8_t)NOVA_PM_CONNECTOR);                         
    #endif

    // Initialize UART peripheral with driver baudrate
    platform_initialize_UART(nova_PM_Header, nova_PM_baudrate);
    uarthandle = nova_PM_Header.uartHandle;
    
    HAL_UART_Receive_IT(uarthandle, (uint8_t *)tmpbuf, 10);

    return 1;
}
uint8_t nova_PM_calculate_sensordata(nova_PM_data_TypeDef *data)
{
    data->PM25 = 0;
    data->PM10 = 0;
    data->id = 0;
    for(int i=0; i<10; i++){
        if(tmpbuf[i]== 170 && tmpbuf[i+1]== 192){
            float tmpPM25, tmpPM10;
            tmpPM25 = tmpbuf[i+3]*256;
            tmpPM25 += tmpbuf[i+2];
            tmpPM25 /= 10;
            data->PM25 = tmpPM25;
            tmpPM10 = tmpbuf[i+5]*256;
            tmpPM10 += tmpbuf[i+4];
            tmpPM10 /= 10;
            data->PM10 = tmpPM10;
            data->id= (tmpbuf[i+7]<<8 | tmpbuf[i+6]);
            return 1;
        }
    }
    return 0;
}

void nova_PM_receive_data()
{
    HAL_UART_Receive_IT(uarthandle, (uint8_t *)tmpbuf, 10);
}

uint8_t nova_PM_reset()
{
    HAL_GPIO_TogglePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin);
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(OCTA_STEPUP_GPIO_Port, OCTA_STEPUP_Pin);
}



