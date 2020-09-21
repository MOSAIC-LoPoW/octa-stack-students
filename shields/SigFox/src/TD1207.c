/**
 * \file        TD1207.cpp
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       TD1207
 *
 * \details     Driver for TD1207.
 *
 * \author      J.Bergs johan.bergs@uantwerpen.be, B.Dil <bram.dil@imec-nl.nl>
 * \date        07-2018
 */
#include "platform.h"
#include "TD1207.h"

struct OCTA_header SigFoxHeader;
UART_HandleTypeDef *uarthandle;
struct OCTA_GPIO *TD1207_reset_pin;
uint8_t TD1207_initialized = 0;
uint8_t TD1207_rts = 0;
uint8_t TD1207_echo = 0;
/************************************************************************/
/* Public Methods                                                       */
/************************************************************************/

uint8_t TD1207_Initialize(void)
{
    if (TD1207_initialized)
    {
        return 0;
    }

    printINF("***Initializing Sigfox driver***\r\n");

    #ifndef SIGFOX_CONNECTOR
        printERR("No SIGFOX_CONNECTOR provided in Makefile\r\n");
        return 0;
    #else
        SigFoxHeader = platform_getHeader((uint8_t)SIGFOX_CONNECTOR);
        if(!SigFoxHeader.active)
        {
            printERR("Invalid SIGFOX_CONNECTOR provided in Makefile\r\n");
            return 0;  
        }
        else   
            printINF("SigFox on P%d, initializing UART\r\n", (uint8_t)SIGFOX_CONNECTOR);                         
    #endif

    // Initialize UART peripheral with driver baudrate
    platform_initialize_UART(SigFoxHeader, TD1207_BAUDRATE);

    TD1207_reset_pin = SigFoxHeader.DIO5;
    uarthandle = SigFoxHeader.uartHandle;

    char initString[3];
    initString[0] = 'A';
    initString[1] = 'T';
    initString[2] = '\r';

    uint8_t result = (HAL_UART_Transmit(uarthandle, (uint8_t *)initString, 3, 1000) == HAL_OK);
    
    uint8_t response[7];
    HAL_UART_Receive(uarthandle, (uint8_t *)response, 15, 1000);
    printDBG("Sigfox AT response: '%s'\n", response);
    if(!strstr(response,"OK"))
    {
        printERR("OK not found after AT check\r\n");
        return 0;
    }
    else
        printINF("SigFox Init OK\r\n\r\n");
    TD1207_initialized = 1;

    return result;
}

void TD1207_toggleResetPin(void){
	HAL_GPIO_WritePin(TD1207_reset_pin->PORT, TD1207_reset_pin->PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(TD1207_reset_pin->PORT, TD1207_reset_pin->PIN, GPIO_PIN_SET);
    HAL_Delay(3000);
	TD1207_initialized = 0;
}

uint8_t TD1207_send(uint8_t *bytes, uint8_t length, uint8_t ack)
{
    HAL_UART_Transmit(uarthandle, "AT\r", 3, 1000);
    HAL_Delay(2000);
    if (length > 12)
    {
        printERR("TD1207 ERROR: payload size should not exceed 12 bytes!\n");
        return 0;
    }

    TD1207_rts = 0;
    char cmd[6] = {0};
    sprintf(cmd, "AT$SF=");

    uint8_t result = 0;

    result = 1;
    result = result && (HAL_UART_Transmit(uarthandle, (uint8_t *)cmd, 6, 1000)==HAL_OK); //mpUART->writeBlocking((uint8_t*) cmd, 6);
    for (uint8_t b = 0; b < length; ++b)
    {
        char hexValue[2];
        sprintf(hexValue, "%02x", bytes[b]);
        result = result && (HAL_UART_Transmit(uarthandle, (uint8_t *)hexValue, 2, 1000)==HAL_OK); //mpUART->writeBlocking((uint8_t*) hexValue, 2);
    }

    if (ack)
    {
        char ackCmd[4] = {0};
        sprintf(ackCmd, ",2,1");
        result = result && (HAL_UART_Transmit(uarthandle, (uint8_t *)ackCmd, 4, 1000)==HAL_OK); //mpUART->writeBlocking((uint8_t*) ackCmd, 4);
    }

    char end = '\r';
    result = result && (HAL_UART_Transmit(uarthandle, (uint8_t *)&end, 1, 1000)==HAL_OK); //mpUART->writeBlocking((uint8_t*) &end, 1);
    printINF("Sigfox message sent with payload size %d\r\n", length);
    return result;
}
