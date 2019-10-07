#include "LSM303AGRSensor.h"


/**
 * Sets the I2C interface for the LSM303AGR
 * @param hi2c: the I2C interface
 */
void LSM303AGR_setI2CInterface(I2C_HandleTypeDef *hi2c) {

	LSM303AGR_hi2c = hi2c;
}

/**
 * Read a certain register of the LSM303AGR depending on the value of ACC_MAG:
 * 		- if this value is 0: read out a register of the accelerometer
 * 		- if this value is 1: read out a register of the magnetormeter
 * The standard HAL library values are used for the size of the register and time-out.
 * @param LSM303AGR_reg: the register that needs to be read of the LSM303AGR
 * @param LSM303AGR_data: a pointer where the data of the register needs to be stored
 * @param ACC_MAG: depending on the value a value a register of the accelerometer of magnetometer are read
 * @return returns the status of the I2C transfer
 */
HAL_StatusTypeDef LSM303AGR_readRegister(uint8_t LSM303AGR_reg,
		uint8_t LSM303AGR_data, uint8_t ACC_MAG) {

	if (ACC_MAG == 0) {
		HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	} else {
		HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	}

	return HAL_OK;
}

/**
 * Writes a cerain value to a certain register of the LSM303AGR depending on the value of ACC_MAG:
 *   	- if this value is 0: write to a register of the accelerometer
 * 		- if this value is 1: write to a register of the magnetormeter
 * the standard HAL library values are used for the size of the register and time-out.
 * @param LSM303AGR_reg: the register where data needs to be written to
 * @param LSM303AGR_data: a pointer to the data that needs to be written
 * @param ACC_MAG: depending on the value, a write is performed to the accelerometer of magnetometer
 * @return returns the status of the I2C transfer
 */
HAL_StatusTypeDef LSM303AGR_writeRegister(uint8_t LSM303AGR_reg,
		uint8_t LSM303AGR_data, uint8_t ACC_MAG) {

	if (ACC_MAG == 0) {
		HAL_I2C_Mem_Write(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_data), HAL_MAX_DELAY);
	} else {
		HAL_I2C_Mem_Write(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
				LSM303AGR_reg, I2C_MEMADD_SIZE_8BIT, &LSM303AGR_data,
				sizeof(LSM303AGR_setting), HAL_MAX_DELAY);
	}

	return HAL_OK;

}

/**
 * Initialization of the LSM303AGR accelerometer and magnetometer.
 * The accelerometer is initialized with the following values:
 * 		- Powerdown,
 * 		- X, Y and Z-axes enabled
 * 		- BDU enabled (output registers are not updated until the MSB and LSB have been read)
 *
 * The magnetometer is initialized with the following values:
 * 		- Idle mode,
 * 		- Temperature compensation enabled,
 * 		- Low-power mode enabled,
 * 		- Offset cancelation enabled,
 * 		- Low pass filter enabled,
 * 		- BDU enabled
 */
void LSM303AGR_init() {

	LSM303AGR_setting = LSM303AGR_ACC_ODR_100Hz | LSM303AGR_ACC_X_EN
			| LSM303AGR_ACC_Y_EN | LSM303AGR_ACC_Z_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, LSM303AGR_setting, 0);

	LSM303AGR_setting = LSM303AGR_ACC_BDU_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG4, LSM303AGR_setting, 0);

	LSM303AGR_setting = LSM303AGR_MAG_COMP_TEMP_EN | LSM303AGR_MAG_LP_EN
			| LSM303AGR_MAG_ODR_10HZ;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);

	LSM303AGR_setting = LSM303AGR_MAG_OFF_CANC | LSM303AGR_MAG_LPF;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_B, LSM303AGR_setting, 1);

	LSM303AGR_setting = LSM303AGR_MAG_BDU;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_C, LSM303AGR_setting, 1);

}

/**
 * Resets the LSM303AGR accelerometer
 */
void LSM303AGR_ACC_reset() {
	LSM303AGR_setting = LSM303AGR_ACC_BOOT;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG5, LSM303AGR_setting, 0);
}

/**
 * Resets the LSM303AGR magnetometer
 */
void LSM303AGR_MAG_reset() {

	LSM303AGR_setting = LSM303AGR_MAG_SOFT_RST;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);

}

/**
 * Put the accelerometer in power-down mode
 */
void LSM303AGR_powerDownAccelerometer() {

	LSM303AGR_setting = LSM303AGR_ACC_ODR_POWERDOWN | LSM303AGR_ACC_X_EN
			| LSM303AGR_ACC_Y_EN | LSM303AGR_ACC_Z_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, LSM303AGR_setting, 0);

}

/**
 * Put the magnetometer in idle mode
 */
void LSM303AGR_powerDownMagnetometer() {

	LSM303AGR_setting = LSM303AGR_MAG_COMP_TEMP_EN | LSM303AGR_MAG_LP_EN
			| LSM303AGR_MAG_ODR_10HZ | LSM303AGR_MAG_MD_IDLE;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);
}

/**
 * Wake up the accelerometer from powerdown mode
 */
void LSM303AGR_wakeUpAccelerometer() {

	LSM303AGR_setting = LSM303AGR_ACC_ODR_10HZ | LSM303AGR_ACC_X_EN
			| LSM303AGR_ACC_Y_EN | LSM303AGR_ACC_Z_EN;
	LSM303AGR_writeRegister(LSM303AGR_ACC_CTRL_REG1, LSM303AGR_setting, 0);

}

/**
 * Wake up the magnetometer from idle mode
 */
void LSM303AGR_wakeUpMagnetometer() {

	LSM303AGR_setting = LSM303AGR_MAG_COMP_TEMP_EN | LSM303AGR_MAG_LP_EN
			| LSM303AGR_MAG_ODR_10HZ | LSM303AGR_MAG_MD_CONT;
	LSM303AGR_writeRegister(LSM303AGR_MAG_CFG_REG_A, LSM303AGR_setting, 1);

}

/**
 * Reads the output registers of the accelerometer
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_ACC_readAccelerationData(int32_t *pData) {
	//accRegisterData[6];
	//accRawData[3];

	//magRegisterData[6];
	//magRawData[3];
	//LSM303AGR_ACC_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,	LSM303AGR_ACC_MULTI_READ, I2C_MEMADD_SIZE_8BIT, accRegisterData,sizeof(accRegisterData), HAL_MAX_DELAY);

	accRawData[0] = (accRegisterData[1] << 8)	| accRegisterData[0];
	accRawData[1] = (accRegisterData[3] << 8)| accRegisterData[2];
	accRawData[2] = (accRegisterData[5] << 8)| accRegisterData[4];

	/* Apply proper shift and sensitivity */
	// Normal mode 10-bit, shift = 6 and FS = 2
	pData[0] = (int32_t) (((accRawData[0] >> 6) * 3900 + 500) / 1000);
	pData[1] = (int32_t) (((accRawData[1] >> 6) * 3900 + 500) / 1000);
	pData[2] = (int32_t) (((accRawData[2] >> 6) * 3900 + 500) / 1000);

}

/**
 * Reads the output registers of the magnetometer and applies the sensitivity
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_MAG_readMagneticData(int32_t *pData) {
	//magRegisterData[6];
		//magRawData[3];
		//LSM303AGR_ACC_TEMP_DATA rawData;

	//LSM303AGR_MAG_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
	LSM303AGR_MAG_MULTI_READ, I2C_MEMADD_SIZE_8BIT, magRegisterData,
			sizeof(magRegisterData), HAL_MAX_DELAY);

	magRawData[0] = (magRegisterData[1] << 8)
			| magRegisterData[0];
	magRawData[1] = (magRegisterData[3] << 8)
			| magRegisterData[2];
	magRawData[2] = (magRegisterData[5] << 8)
			| magRegisterData[4];

	/* Calculate the data. */
	pData[0] = (int32_t) (magRawData[0] * 1.5f);
	pData[1] = (int32_t) (magRawData[1] * 1.5f);
	pData[2] = (int32_t) (magRawData[2] * 1.5f);

}

/**
 * Read the output registers of the magnetometer
 * @param pData: a pointer to where the data needs to be stored
 */
void LSM303AGR_MAG_readMagneticRawData(uint16_t *pData) {
	//magRegisterData[6];
			//magRawData[3];
	//LSM303AGR_MAG_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_MAG_I2C_ADDRESS,
	LSM303AGR_MAG_MULTI_READ, I2C_MEMADD_SIZE_8BIT, magRegisterData,
			sizeof(magRegisterData), HAL_MAX_DELAY);

	pData[0] = (magRegisterData[1] << 8)
			| magRegisterData[0];
	pData[1] = (magRegisterData[3] << 8)
			| magRegisterData[2];
	pData[2] = (magRegisterData[5] << 8)
			| magRegisterData[4];


}

void LSM303AGR_ACC_readAccelerationRawData(uint16_t *pData) {
	//accRegisterData[6];
	//accRawData[3];

	//magRegisterData[6];
	//magRawData[3];
	//LSM303AGR_ACC_TEMP_DATA rawData;

	HAL_I2C_Mem_Read(LSM303AGR_hi2c, LSM303AGR_ACC_I2C_ADDRESS,	LSM303AGR_ACC_MULTI_READ, I2C_MEMADD_SIZE_8BIT, accRegisterData,sizeof(accRegisterData), HAL_MAX_DELAY);

	pData[0] = (accRegisterData[1] << 8)	| accRegisterData[0];
	pData[1] = (accRegisterData[3] << 8)| accRegisterData[2];
	pData[2] = (accRegisterData[5] << 8)| accRegisterData[4];

	/* Apply proper shift and sensitivity */
	// Normal mode 10-bit, shift = 6 and FS = 2
	//pData[0] = (int32_t) (((accRawData[0] >> 6) * 3900 + 500) / 1000);
	//pData[1] = (int32_t) (((accRawData[1] >> 6) * 3900 + 500) / 1000);
	//pData[2] = (int32_t) (((accRawData[2] >> 6) * 3900 + 500) / 1000);

}
