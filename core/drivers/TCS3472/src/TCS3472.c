/*
 * TCS3472.c
 *
 *  Created on: 1 Mar 2018
 *    
 */
#include "TCS3472.h"
#include "platform.h"
#define SLAVE_ADDRESS           0x29

#define ENABLE                 0x00
#define ATIME                   0x01
#define WTIME                   0x03
#define AILTL                   0x04
#define AIHTL                   0x06
#define PERS                    0x0C
#define CONFIG                  0x0D
#define CONTROL                 0x0F
#define ID                      0x12
#define STATUS                  0x13
#define CDATA                   0x14
#define RDATA                   0x16
#define GDATA                   0x18
#define BDATA                   0x1A

void TC3472_init(I2C_HandleTypeDef *hi2c)
{
	hi2cLib=hi2c;
    enablePowerAndRGBC();
	setIntegrationTime(100);
}
uint8_t WRITE_REGISTER_TC3472(uint8_t pData[],uint8_t length)
{
	uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, SLAVE_ADDRESS<<1, pData,length, HAL_MAX_DELAY);
	return status;
}
uint8_t READ_REGISTER_TC3472(uint8_t buf[],uint8_t reg,uint8_t length)
{
	uint8_t status = HAL_I2C_Mem_Read(hi2cLib, SLAVE_ADDRESS<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
	return status;
}


int writeSingleRegister( uint8_t address, uint8_t data ){
    uint8_t tx[2] = { address | 160, data }; //0d160 = 0b10100000
    //int ack = i2c.write( SLAVE_ADDRESS << 1, tx, 2 );
    int ack=WRITE_REGISTER_TC3472(tx,2);
    return ack;
}

int writeMultipleRegisters( uint8_t address, uint8_t* data, int quantity ){
    uint8_t tx[ quantity + 1 ];
    tx[0] = address | 160;
    for ( int i = 1; i <= quantity; i++ ){
        tx[ i ] = data[ i - 1 ];
    }
    //int ack = i2c.write( SLAVE_ADDRESS << 1, tx, quantity + 1 );
    int ack=WRITE_REGISTER_TC3472(tx,quantity + 1);
    return ack;
}

uint8_t readSingleRegister( uint8_t address ){
    //uint8_t output = 255;
    uint8_t hwID[1];
    uint8_t command = address | 160; //0d160 = 0b10100000
    READ_REGISTER_TC3472(hwID,command,1);
    //i2c.write( SLAVE_ADDRESS << 1, &command, 1, true );
    //i2c.read( SLAVE_ADDRESS << 1, &output, 1 );
    return hwID[0];
}

int  readMultipleRegisters( uint8_t address, uint8_t* output, int quantity ){
	uint8_t command = address | 160; //0d160 = 0b10100000
    int ack=READ_REGISTER_TC3472(output,command,quantity);
    //i2c.write( SLAVE_ADDRESS << 1, &command, 1, true );
    //int ack = i2c.read( SLAVE_ADDRESS << 1, output, quantity );
    return ack;
}

void  getAllColors( uint16_t* readings ){
    uint8_t buffer[8] = { 0 };

    readMultipleRegisters( CDATA, buffer, 8 );

    readings[0] = (uint16_t)buffer[1] << 8 | (uint16_t)buffer[0];
    readings[1] = (uint16_t)buffer[3] << 8 | (uint16_t)buffer[2];
    readings[2] = (uint16_t)buffer[5] << 8 | (uint16_t)buffer[4];
    readings[3] = (uint16_t)buffer[7] << 8 | (uint16_t)buffer[6];
}

int  getClearData(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( CDATA, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  getRedData(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( RDATA, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  getGreenData(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( GDATA, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  getBlueData(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( BDATA, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  enablePower(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old | 1; // sets PON (bit 0) to 1
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  disablePower(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old & 254; // sets PON (bit 0) to 0
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

uint8_t  isPowerEnabled(){
    uint8_t enable = readSingleRegister( ENABLE );
    uint8_t pon = enable << 7;
    pon = pon >> 7; // gets PON (bit 0) from ENABLE register byte
    return (uint8_t)pon;
}

int  enableRGBC(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old | 2; // sets AEN (bit 1) to 1
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  disableRGBC(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old & 253; // sets AEN (bit 1) to 0
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

uint8_t  isRGBCEnabled(){
    uint8_t enable = readSingleRegister( ENABLE );
    uint8_t aen = enable << 6;
    aen = aen >> 7; // gets AEN (bit 1) from ENABLE register byte
    return (uint8_t)aen;
}

int  enablePowerAndRGBC(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old | 3; // sets PON (bit 0) and AEN (bit 1) to 1
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  disablePowerAndRGBC(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old & 252; // sets PON (bit 0) and AEN (bit 1) to 0
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  enableWait(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old | 8; // sets WEN (bit 3) to 1
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  disableWait(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old & 247; // sets WEN (bit 3) to 0
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

uint8_t  isWaitEnabled(){
    uint8_t enable = readSingleRegister( ENABLE );
    uint8_t wen = enable << 4;
    wen = wen >> 7; // gets WEN (bit 3) from ENABLE register byte
    return (uint8_t)wen;
}

int  enableInterrupt(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old | 16; // sets AIEN (bit 4) to 1
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

int  disableInterrupt(){
    uint8_t enable_old = readSingleRegister( ENABLE );
    uint8_t enable_new = enable_old & 239; // sets AIEN (bit 4) to 0
    int ack = writeSingleRegister( ENABLE, enable_new );
    return ack;
}

uint8_t  isInterruptEnabled(){
    uint8_t enable = readSingleRegister( ENABLE );
    uint8_t aien = enable << 3;
    aien = aien >> 7; // gets AIEN (bit 4) from ENABLE register byte
    return (uint8_t)aien;
}

int  setIntegrationTime( const float itime ){
    uint8_t atime = 256 - roundTowardsZero( itime / 2.4 ); // rounding ensures nearest value of atime is used
    int ack = writeSingleRegister( ATIME, atime );
    return ack;
}

float  readIntegrationTime(){
    float itime = 0;
    uint8_t atime = readSingleRegister( ATIME );
    itime = 2.4 * ( 256 - atime );
    return itime;
}

int  setWaitTime( const float time ){
    int ack = 1;
    uint8_t wtime = 0;
    if ( time >= 2.39 && time <= 614.4 ){ // 2.39 instead of 2.4 to allow for float accuracy errors
        ack = writeSingleRegister( CONFIG, 0 ); // sets WLONG to 0
        wtime = 256 - roundTowardsZero( time / 2.4 );
    }
    else if ( time > 614.4 && time <= 7400.1 ){ // 7400.1 instead of 7400 to allow for float accuracy errors
        ack = writeSingleRegister( CONFIG, 2 ); // sets WLONG to 1
        wtime = 256 - roundTowardsZero( time / 28.8 );
    }
    ack = ack || writeSingleRegister( WTIME, wtime );
    return ack;
}

float  readWaitTime(){
    float time = 0;
    uint8_t wtime = readSingleRegister( WTIME );
    uint8_t config = readSingleRegister( CONFIG );
    int wlong = ( config << 6 ) >> 7; // gets WLONG (bit 1) from CONFIG register byte
    if ( wlong == 0 ){
        time = 2.4 * ( 256 - wtime );
    }
    else if ( wlong == 1 ){
        time = 28.8 * ( 256 - wtime ); // 28.8 = 2.4 * 12
    }
    return time;
}

uint8_t  readEnableRegister(){
    return readSingleRegister( ENABLE );
}

int  readLowInterruptThreshold(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( AILTL, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  readHighInterruptThreshold(){
    uint8_t buffer[2] = { 0 };
    readMultipleRegisters( AIHTL, buffer, 2 );
    int reading = (int)buffer[1] << 8 | (int)buffer[0];
    return reading;
}

int  setLowInterruptThreshold( const int threshold ){
    uint8_t threshold_bytes[2];
    threshold_bytes[0] = threshold; // take lowest 8 bits of threshold
    threshold_bytes[1] = threshold >> 8; // take highest 8 bits of threshold
    int ack = writeMultipleRegisters( AILTL, threshold_bytes, 2 );
    return ack;
}

int  setHighInterruptThreshold( const int threshold ){
    uint8_t threshold_bytes[2];
    threshold_bytes[0] = threshold;
    threshold_bytes[1] = threshold >> 8;
    int ack = writeMultipleRegisters( AIHTL, threshold_bytes, 2 );
    return ack;
}

int  readInterruptPersistence(){
    uint8_t pers = readSingleRegister( PERS );
    uint8_t persistence_bits = ( pers << 4 ) >> 4; // discard bits 4 to 7, keep only bits 0 to 3
    int persistence = -1;
    switch (persistence_bits){
        case 0:
            persistence = 0;
            break;
        case 1:
            persistence = 1;
            break;
        case 2:
            persistence = 2;
            break;
        case 3:
            persistence = 3;
            break;
        case 4:
            persistence = 5;
            break;
        case 5:
            persistence = 10;
            break;
        case 6:
            persistence = 15;
            break;
        case 7:
            persistence = 20;
            break;
        case 8:
            persistence = 25;
            break;
        case 9:
            persistence = 30;
            break;
        case 10:
            persistence = 35;
            break;
        case 11:
            persistence = 40;
            break;
        case 12:
            persistence = 45;
            break;
        case 13:
            persistence = 50;
            break;
        case 14:
            persistence = 55;
            break;
        case 15:
            persistence = 60;
            break;
        default:
            break;
    }
    return persistence;
}

int  setInterruptPersistence( const int persistence ){
    uint8_t pers_byte;
    int ack = 0;
    switch (persistence){
        case 0:
            pers_byte = 0;
            break;
        case 1:
            pers_byte = 1;
            break;
        case 2:
            pers_byte = 2;
            break;
        case 3:
            pers_byte = 3;
            break;
        case 5:
            pers_byte = 4;
            break;
        case 10:
            pers_byte = 5;
            break;
        case 15:
            pers_byte = 6;
            break;
        case 20:
            pers_byte = 7;
            break;
        case 25:
            pers_byte = 8;
            break;
        case 30:
            pers_byte = 9;
            break;
        case 35:
            pers_byte = 10;
            break;
        case 40:
            pers_byte = 11;
            break;
        case 45:
            pers_byte = 12;
            break;
        case 50:
            pers_byte = 13;
            break;
        case 55:
            pers_byte = 14;
            break;
        case 60:
            pers_byte = 15;
            break;
        default:
            ack = 2; // 2 used to indicate invalid entry
            break;
    }
    if ( ack != 2 ){
        ack = writeSingleRegister( PERS, pers_byte );
    }
    return ack;
}

int  clearInterrupt(){
    uint8_t tx[1];
    tx[0]= 230;
    int ack=WRITE_REGISTER_TC3472(tx,1);
    //int ack = i2c.write( SLAVE_ADDRESS << 1, &tx, 1 );
    return ack;
}

int  readRGBCGain(){
    uint8_t control = readSingleRegister( CONTROL );
    uint8_t gain_bits = ( control << 6 ) >> 6; // discard  bits 2 to 7, keep only bits 0 & 1
    int gain;
    switch (gain_bits) {
        case 0:
            gain = 1;
            break;
        case 1:
            gain = 4;
            break;
        case 2:
            gain = 16;
            break;
        case 3:
            gain = 60;
            break;
        default:
            gain = 0;
            break;
    }
    return gain;
}

int  setRGBCGain( const int gain ){
    uint8_t control;
    int ack = 0;
    switch (gain){
        case 1:
            control = 0;
            break;
        case 4:
            control = 1;
            break;
        case 16:
            control = 2;
            break;
        case 60:
            control = 3;
            break;
        default:
            ack = 2; // 2 used to indicate invalid entry
            break;
    }
    if ( ack != 2 ){
        ack = writeSingleRegister( CONTROL, control );
    }
    return ack;
}

uint8_t  getDeviceID(){
    return readSingleRegister( ID );
}

uint8_t  readStatusRegister(){
    return readSingleRegister( STATUS );
}

float  roundTowardsZero( const float value ){
    float result = 0;
    if ( ( value >= 0 && ( value - (int)value ) < 0.5 ) || ( value < 0 && ( abs(value) - (int)abs(value) ) >= 0.5 ) ){
        result = floor(value);
    }
    else{
        result = ceil(value);
    }
    return result;
}

void turnOnLED(){
    #ifndef platform_octa_stm
        HAL_GPIO_WritePin(OCTA_LIGHTSENSORLED_Port, OCTA_LIGHTSENSORLED_Pin, 1);
    #endif
}

void turnOffLED(){
    #ifndef platform_octa_stm
        HAL_GPIO_WritePin(OCTA_LIGHTSENSORLED_Port, OCTA_LIGHTSENSORLED_Pin, 0);
    #endif
}