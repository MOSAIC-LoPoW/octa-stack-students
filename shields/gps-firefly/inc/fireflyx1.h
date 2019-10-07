/*
 * fireflyx1.h
 *
 *  Created on: 16 May 2018
 *      Author: Fifth
 */
#include "stm32l4xx_hal.h"
#include "nmea.h"

#ifndef FIREFLYX1_H_
#define FIREFLYX1_H_
#define GPS_BUFFER_SIZE 255
#define MT3333_ADDR         0x20

#define gps_position_t    nmea_position_t
#define gps_have_position nmea_have_position
#define gps_get_position  nmea_get_position

typedef struct gps_position_dd {
  float latitude;
  float longitude;
  float hdop;
} gps_position_dd_t;

union {
			float f;
			struct {
				uint8_t b1, b2, b3, b4;
			} bytes;
		} currentLatitude, currentLongitude, currentHdop;

gps_position_dd_t gps_get_position_dd(void);
void Firefly_setI2CHandle(I2C_HandleTypeDef *hi2c);
uint8_t Firefly_Initialize(void);
uint8_t Firefly_communication_check(void);
uint8_t READ_REGISTER_GPS(uint8_t buf[],uint16_t reg,uint8_t length);
uint8_t WRITE_REGISTER_GPS(uint8_t pData[],uint8_t length);
uint8_t READ_REG_GPS(uint8_t buf[],uint8_t length);
uint8_t Firefly_receive(uint8_t response[]);
uint8_t feedToProcessor(uint8_t response[]);
uint8_t Firefly_send(const char* cmd);
void Firefly_sleep(void);
static uint16_t _compute_checksum(uint8_t* bytes, uint8_t length);

#endif /* FIREFLYX1_H_ */
