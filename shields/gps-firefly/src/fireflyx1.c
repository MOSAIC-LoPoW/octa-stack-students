/*
 * fireflyx1.c
 *
 *  Created on: 16 May 2018
 *      Author: Fifth
 */
#include "platform.h"
#include "fireflyx1.h"

static uint8_t _gps_buffer[GPS_BUFFER_SIZE] = { 0 };
static uint8_t _active = 0;
static uint8_t wait_pmtk_ack = 0;
static uint32_t baudrate = 115200;       // set to 57600 or higher for updaterates higher than 1Hz
static uint8_t baudrate_set = 0;
static uint16_t updaterate = 500;       // 1000 = 1Hz, 500 = 2Hz, 200 = 5Hz, 100 = 10Hz (best to use max 2Hz)
static uint8_t updateRate_set = 0;
struct OCTA_header FireflyHeader;
I2C_HandleTypeDef *hi2cLib;
struct OCTA_GPIO *firefly_int_pin; //DIO1
struct OCTA_GPIO *firefly_reset_pin; //DIO6

uint8_t Firefly_Initialize(void)
{
    printf("***Initializing Firefly-GPS driver***\r\n");

    #ifndef FIREFLY_CONNECTOR
        printf("No FIREFLY_CONNECTOR provided in Makefile\r\n");
        return 0;
    #else
        FireflyHeader = platform_getHeader((uint8_t)FIREFLY_CONNECTOR);
        if(!FireflyHeader.active)
        {
            printf("Invalid FIREFLY_CONNECTOR provided in Makefile\r\n");
            return 0;  
        }
        else   
            printf("Firefly on P%d, initializing I2C\r\n", (uint8_t)FIREFLY_CONNECTOR);                         
    #endif

    // Initialize I2C peripheral with driver baudrate
    platform_initialize_I2C(FireflyHeader);
    
    hi2cLib = FireflyHeader.i2cHandle;
    firefly_int_pin = FireflyHeader.DIO1;
    firefly_reset_pin = FireflyHeader.DIO6;

    //TODO: gps communication check
    if(!Firefly_communication_check())
    {
        printf("Firefly GPS Init NOK\r\n\r\n");
        return 0;
    }
    printf("Firefly GPS Init OK\r\n\r\n");
    return 1;
    
}

uint8_t Firefly_communication_check(void)
{  
    uint8_t buffer[10];
    if(!READ_REG_GPS(buffer,10))
    {
        printf("Firefly communication check OK\r\n");
        return 1;
    }
    else
    {
        printf("Firefly communication check NOK\r\n");
        return 0;
    }
}

static uint16_t Firefly_compute_checksum(uint8_t* bytes, uint8_t length)
{
    uint8_t sum = 0;

    do {
      sum ^= *bytes++;
    } while(--length);

    return sum;
}

uint8_t Firefly_send(const char* cmd)
{
    uint8_t length = strlen(cmd);  // construct entire cmd buffer
    uint8_t buffer[255] = { '$', 'P', 'M', 'T', 'K' }; // copy in cmd
    memcpy(&buffer[5], cmd, length); // add '*'
    buffer[5+length] = '*';  // add checksum
    uint16_t checksum = _compute_checksum(&buffer[1], 4+length);
    buffer[5+length+3] = '\r';
    buffer[5+length+4] = '\n';
    WRITE_REGISTER_GPS(buffer,5+length+5);
    return 1;
}

uint8_t Firefly_sendDefault(void)
{
    //uint8_t buffer[18] = { '$', 'P', 'M', 'T', 'K' ,'2','2','0',',','1','0','0','0','*','1','F'}; // copy in cmd
    uint8_t buffer[18] = { '$', 'P', 'M', 'T', 'K' ,'1','0','3','*','3','0'}; // copy in cmd
    buffer[11] = '\r';
    buffer[12] = '\n';
    WRITE_REGISTER_GPS(buffer,12);

    return 1;
}
uint8_t WRITE_REGISTER_GPS(uint8_t pData[],uint8_t length)
{
    uint8_t status=HAL_I2C_Master_Transmit(hi2cLib, MT3333_ADDR<<1, pData,length, HAL_MAX_DELAY);
    return status;
}
uint8_t READ_REGISTER_GPS(uint8_t buf[],uint16_t reg,uint8_t length)
{
    uint8_t status = HAL_I2C_Mem_Read(hi2cLib, MT3333_ADDR<<1, reg, I2C_MEMADD_SIZE_8BIT, buf, length, HAL_MAX_DELAY);
    return status;
}
uint8_t READ_REG_GPS(uint8_t buf[],uint8_t length)
{
    uint8_t status= HAL_I2C_Master_Receive(hi2cLib, MT3333_ADDR, buf, length, HAL_MAX_DELAY);
    return status;
}
uint8_t Firefly_receive(uint8_t response[])
{
    READ_REG_GPS(response,1000);
    return feedToProcessor(response);
}

void Firefly_sleep(void)
{
    uint8_t buffer[15] = { '$', 'P', 'M', 'T', 'K' , '1' , '6' , '1' , ',' , '0' , '*' , '2' , '8' , '\r' , '\n' };
    WRITE_REGISTER_GPS(buffer,15);
}

uint8_t feedToProcessor(uint8_t response[])
{
    uint8_t i;
    for(i = 0; i < 255; i++)
    {
      nmea_parse(response[i]);
    }
    return nmea_have_position();
}

gps_position_dd_t gps_get_position_dd() {
    gps_position_dd_t dd;
    nmea_position_t   pos = nmea_get_position();
    dd.latitude = pos.latitude.deg + pos.latitude.min/60.0;
    if(pos.latitude.ns == 's') {
      dd.latitude *= -1;
    }
    dd.longitude = pos.longitude.deg + pos.longitude.min/60.0;
    if(pos.longitude.ew == 'w') {
      dd.longitude *= -1;
    }
    dd.hdop = pos.hdop;
    return dd;
}
