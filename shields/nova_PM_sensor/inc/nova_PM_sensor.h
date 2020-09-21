/**
 * \file        nova_PM_sensor.h
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


#include "stm32l4xx_hal.h"

#define nova_PM_baudrate	9600
typedef struct  {
  float PM25;				/* PM2.5*/		
  float PM10;			
  uint16_t id;            
 } nova_PM_data_TypeDef;

/*
* Initializes the sensor using its default configuration
*/
uint8_t nova_PM_Initialize(void);
void nova_PM_receive_data(void);
uint8_t nova_PM_reset(void);
uint8_t nova_PM_calculate_sensordata(nova_PM_data_TypeDef *data);
