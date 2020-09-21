#include "stm32l4xx_hal.h"

#define ROD_OK 0                      //All is well
#define ROD_E_ROD_NOT_FOUND -1        //NO UART data coming in
#define ROD_E_INVALID_DATA -2         //ROD sent invalid data (eg junkbytes)
#define ROD_E_ROD_ERROR -3            //ROD error field is not 0
#define ROD_E_INVALID_CONDUCTIVITY -4 //Phase of the impedance is too high

#define ROD_BAUDRATE 9600

/**
 * struct    RODv1Data
 * brief     Contains the data of a single ROD measurement
 */
struct RODData {
    //Fields from string
    int32_t sensorID;
    int32_t impedance; //ohm
    int16_t phase; //Deg
    uint8_t error; //0 is OK
    int32_t voltageA;
    int32_t voltageB;
    int32_t voltageC;
    int16_t temp; //Celcius
    //GROW parsed fields below, might be different for IOW
    int pH; //pH
    float conductivity; //microSiemens/cm
};


uint8_t ROD_Initialize(void);
int ROD_getData(struct RODData *data);
int ROD_poll(void);
int ROD_parseData(struct RODData *data);
uint8_t ROD_parseInt(uint8_t *index, int *val);
void ROD_Enable();
void ROD_Disable();

      