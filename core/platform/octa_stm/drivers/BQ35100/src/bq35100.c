#include "bq35100.h"
#include "platform.h"

static I2C_HandleTypeDef *hi2cLib;

void bq35100_init(I2C_HandleTypeDef *hi2c)
{
    hi2cLib = hi2c;
	HAL_StatusTypeDef ret  = 0;
	uint8_t buf[10];

	//ENABLE GE
	HAL_GPIO_WritePin(OCTA_GAUGE_ENABLE_Port, OCTA_GAUGE_ENABLE_Pin, GPIO_PIN_SET);

	//WAIT FOR SOMETIME
	HAL_Delay(1000);

	//ENABLE ACC MODE IN CONFIG A- this is the default mode if no configurations changes were made
	buf[0] = 0x3E; //MACAccessControl
	buf[1] = 0xB1; //LSB of Config A address
	buf[2] = 0x41; // MSB Config A
	buf[3] = 0x00; // Config A value

	//WRITE TO CONFIG  A
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 4, HAL_MAX_DELAY);

	buf[0] = 0x60; //MACDataSum
	buf[1] = 0x0D;	//Checksum = NOT(address + data)
	buf[2] = 0x05;	//Length = (Address + data + checksum +len)

	//WRITE CHECKSUM TO MACDataSum
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 3, HAL_MAX_DELAY);

	//READ BACK Config A to confirm write (only for testing )
	buf[0] = 0x3E; //
	buf[1] = 0xB1; //
	buf[2] = 0x41;

	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 3, HAL_MAX_DELAY);

	HAL_Delay(1);

	buf[0] = 0x40; //MACData will have the read out data

	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);

	HAL_Delay(100);

	//Disable Guage
	HAL_GPIO_WritePin(OCTA_GAUGE_ENABLE_Port, OCTA_GAUGE_ENABLE_Pin, GPIO_PIN_RESET);

	HAL_Delay(1000);

	//ENABLE GAUGE
	HAL_GPIO_WritePin(OCTA_GAUGE_ENABLE_Port, OCTA_GAUGE_ENABLE_Pin, GPIO_PIN_SET);

	HAL_Delay(300);

	//START GUAGE
	buf[0] = 0x3E; //control command
	buf[1] = 0x11; //start (LSB)
	buf[2] = 0x00; //MSB

	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 3, HAL_MAX_DELAY);
	HAL_Delay(1000);

	//READ STATUS to clear ALERT
	buf[0] = 0x0A;
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib, 0xAA, buf, 1, HAL_MAX_DELAY);
	HAL_Delay(1);
}

/**
 * Read accumulated capacity- non-volatile storage
 * Enable Life time Counting if requires a Accumulated capacity to be non-volatile
 */
void bq35100_read_acc_data(bq35100_data_struct* bq35100_data)
{
	HAL_StatusTypeDef ret  = 0;
	uint8_t buf[10];

	//READ VOLTAGE
	buf[0] = 0x08; //Read voltage command

	uint8_t voltbuff[2];
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib,0xAA, (uint8_t *)voltbuff, 2, HAL_MAX_DELAY);
	int16LittleEndian.byte[0] = voltbuff[0];
	int16LittleEndian.byte[1] = voltbuff[1];
	bq35100_data->volt = int16LittleEndian.integer;

	HAL_Delay(1);

	//READ INTERNAL TEMP: default is external, I am using internal temp
	buf[0] = 0x28; //Read internal temp command

	uint8_t tempbuff[2];
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib,0xAA, (uint8_t *)tempbuff, 2, HAL_MAX_DELAY);
	uint16LittleEndian.byte[0] = tempbuff[0];
	uint16LittleEndian.byte[1] = tempbuff[1];
	bq35100_data->temp = uint16LittleEndian.integer;

	HAL_Delay(1);

	//READ ACCUMULATED CAPACITY
	buf[0] = 0x02; //Read acc capacity command

	uint8_t accbuff[4];
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib,0xAA, (uint8_t *)accbuff, 4, HAL_MAX_DELAY);
	int32LittleEndian.byte[0] = accbuff[0];
	int32LittleEndian.byte[1] = accbuff[1];
	int32LittleEndian.byte[2] = accbuff[2];
	int32LittleEndian.byte[3] = accbuff[3];
	bq35100_data->acc_capacity = int32LittleEndian.integer;

	HAL_Delay(1);

	//READ CURRENT
	buf[0] = 0x0C; //read current

	uint8_t currentbuff[2];
	ret = HAL_I2C_Master_Transmit(hi2cLib,0xAA, buf, 1, HAL_MAX_DELAY);
	ret = HAL_I2C_Master_Receive(hi2cLib,0xAA, (uint8_t *)currentbuff, 2, HAL_MAX_DELAY);
	int16LittleEndian.byte[0] = currentbuff[0];
	int16LittleEndian.byte[1] = currentbuff[1];
	bq35100_data->current = int16LittleEndian.integer;

	HAL_Delay(1);

    return;
}
