/*
 * sht31.c
 *
 *  Created on: 7 Mar 2018
 *      Author: Fifth
 */

#include "sht31.h"

#include <stdio.h>
#include <math.h>
#define SHT31_DEFAULT_ADDR    0x44

void setI2CInterface_SHT31(I2C_HandleTypeDef *hi2c)
{
	SHThi2cLib=hi2c;
}
uint8_t WRITE_REGISTER_SHT31(uint8_t pData[],uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(SHThi2cLib, SHT31_DEFAULT_ADDR<<1, pData,length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REGISTER_SHT31(uint8_t buf[],uint16_t reg,uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(SHThi2cLib, SHT31_DEFAULT_ADDR<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REG_SHT31(uint8_t buf[],uint8_t length)
{
	uint8_t status= HAL_I2C_Master_Receive(SHThi2cLib, SHT31_DEFAULT_ADDR<<1, buf, length, HAL_MAX_DELAY);
return status;
}

uint8_t  SHT31_begin() {
	SHT31_reset();
  //return (readStatus() == 0x40);
  return 1;
}
void SHT31_get_temp_hum(float* sht31Data)
{
	readTempHum();
	sht31Data[1]= humidity;
	sht31Data[0]= temp;
}
void  writeCommand(uint16_t cmd) {
	uint8_t pData[2];
	pData[0]=cmd >> 8;
	pData[1]=cmd & 0xFF;
	WRITE_REGISTER_SHT31(pData,2);

}

uint16_t  readStatus(void) {
	uint8_t stat[2];
	writeCommand(0xF32D);
	HAL_Delay(10);
	READ_REG_SHT31(stat,3);
	uint16_t stat2=(stat[0]<<8)|stat[1];
	return stat2;
}

void  SHT31_reset(void) {
  writeCommand(SHT31_SOFTRESET);
}

void  heater(uint8_t h) {
  if (h)
    writeCommand(SHT31_HEATEREN);
  else
    writeCommand(SHT31_HEATERDIS);
}


float  readTemperature(void) {
  if (! readTempHum()) return 0;

  return temp;
}


float  readHumidity(void) {
  if (! readTempHum()) return 0;

  return humidity;
}


uint8_t  readTempHum(void)
{
  uint8_t readbuffer[6];

  writeCommand(SHT31_MEAS_HIGHREP);
  HAL_Delay(50);
  READ_REG_SHT31(readbuffer,6);


  uint16_t ST, SRH;
  ST = readbuffer[0];
  ST <<= 8;
  ST |= readbuffer[1];



  SRH = readbuffer[3];
  SRH <<= 8;
  SRH |= readbuffer[4];



 // Serial.print("ST = "); Serial.println(ST);
  double stemp = ST;
  stemp *= 175;
  stemp /= 0xffff;
  stemp = -45 + stemp;
  temp = stemp;

//  Serial.print("SRH = "); Serial.println(SRH);
  double shum = SRH;
  shum *= 100;
  shum /= 0xFFFF;

  humidity = shum;

  return 1;
}

uint8_t  readTempHum2(uint16_t* sensorData)
{
  uint8_t readbuffer[6];

  writeCommand(SHT31_MEAS_HIGHREP);
  HAL_Delay(50);
  READ_REG_SHT31(readbuffer,6);


  uint16_t ST, SRH;
  ST = readbuffer[0];
  ST <<= 8;
  ST |= readbuffer[1];

  SRH = readbuffer[3];
  SRH <<= 8;
  SRH |= readbuffer[4];

  sensorData[0]=ST;

  sensorData[1]=SRH;

  return 1;
}



uint8_t  crc8(const uint8_t *data, int len)
{
/*
*
 * CRC-8 formula from page 14 of SHT spec pdf
 *
 * Test data 0xBE, 0xEF should yield 0x92
 *
 * Initialization data 0xFF
 * Polynomial 0x31 (x8 + x5 +x4 +1)
 * Final XOR 0x00
 */
/*
  uint8_t POLYNOMIAL(0x31);

  uint8_t crc(0xFF);

  for ( int j = len; j; --j ) {
      crc ^= *data++;

      for ( int i = 8; i; --i ) {
	crc = ( crc & 0x80 )
	  ? (crc << 1) ^ POLYNOMIAL
	  : (crc << 1);
      }
  }
  return crc;
  */
	return 1;
}
