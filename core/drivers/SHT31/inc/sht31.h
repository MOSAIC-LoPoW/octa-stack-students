#include "stm32l4xx_hal.h"
#ifndef SHT31_H_
#define SHT31_H_


#define SHT31_MEAS_HIGHREP_STRETCH 0x2C06
#define SHT31_MEAS_MEDREP_STRETCH  0x2C0D
#define SHT31_MEAS_LOWREP_STRETCH  0x2C10
#define SHT31_MEAS_HIGHREP         0x2400
#define SHT31_MEAS_MEDREP          0x240B
#define SHT31_MEAS_LOWREP          0x2416
#define SHT31_READSTATUS           0xF32D
#define SHT31_CLEARSTATUS          0x3041
#define SHT31_SOFTRESET            0x30A2
#define SHT31_HEATEREN             0x306D
#define SHT31_HEATERDIS            0x3066

I2C_HandleTypeDef *SHThi2cLib;
void setI2CInterface_SHT31(I2C_HandleTypeDef *hi2c);
uint8_t READ_REGISTER_SHT31(uint8_t buf[],uint16_t reg,uint8_t length);
uint8_t WRITE_REGISTER_SHT31(uint8_t pData[],uint8_t length);
uint8_t SHT31_begin();
float readTemperature(void);
float readHumidity(void);
uint16_t readStatus(void);
void SHT31_reset(void);
void heater(uint8_t h);
uint8_t crc8(const uint8_t *data, int len);
void SHT31_get_temp_hum(float* sht31Data);
uint8_t readTempHum(void);
uint8_t  readTempHum2(uint16_t* sensorData);
void writeCommand(uint16_t cmd);
uint8_t _i2caddr;
uint8_t readData(void);
float humidity, temp;

#endif /* SHT31_H_ */
