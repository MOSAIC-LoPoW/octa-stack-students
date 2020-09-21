#include "platform.h"
#include "ROD.h"
#include <math.h>

#define ROD_MAX_RETRIES 5
#define ROD_MAX_MESSAGE_SIZE 100
#define ROD_IMP_KCELL 0.8
#define ROD_MAX_PHASE 15

//Buffer, used to store the incoming data
uint8_t _buff[ROD_MAX_MESSAGE_SIZE];

struct OCTA_header RODHeader;
struct OCTA_GPIO *ROD_enable_pin;

UART_HandleTypeDef *_uart;

uint8_t ROD_initialised = 0;

int32_t ROD_sensorID = 0;

  /**
   * \brief    Initializes the ROD driver.
   * \param    aUART           IUART driver.
   * \returns  true is everything went OK.
   */
  uint8_t ROD_Initialize(void)
  {

    if(ROD_initialised) {
      return 0;
    }

    printINF("***Initializing ROD driver***\r\n");

    //check if platform is octa stm with onboad rs232 chip
    #ifdef platform_octa_stm
      printINF("Platform octa-stm, using RS232_UART\r\n");
      RS232_UART_Init(ROD_BAUDRATE);
      _uart = &RS232_UART;
    //else check connector
    #else
      #ifndef ROD_CONNECTOR
          printERR("No ROD_CONNECTOR provided in Makefile\r\n");
          return 0;
      #else
          RODHeader = platform_getHeader((uint8_t)ROD_CONNECTOR);
          if(!RODHeader.active)
          {
              printERR("Invalid ROD_CONNECTOR provided in Makefile\r\n");
              return 0;
          }
          else
              printINF("ROD on P%d, initializing UART\r\n\r\n", (uint8_t)ROD_CONNECTOR);
      #endif

      // Initialize UART peripheral with driver baudrate
      platform_initialize_UART(RODHeader, ROD_BAUDRATE);
      ROD_enable_pin = RODHeader.DIO1;
      _uart = RODHeader.uartHandle;
    #endif

    ROD_Disable();

    ROD_initialised = 1;
    return ROD_initialised;
  }


  /**
   * \brief    Fills the given RODData struct.
   * \param    data    The ROD struct to fill.
   * \returns  ROD_OK if successful, error is not
   */
  int ROD_getData(struct RODData *data) {

    //ROD_Enable();
    int output = ROD_OK;
    
    for(int n = 0; n < ROD_MAX_RETRIES; n++ ) {
      //Get the data
      IWDG_feed(NULL);
      output = ROD_poll();
      if(output != ROD_OK) {
        //Try again.
        continue;
      }
      //There's data in the buffer, let's try to parse it.
      output = ROD_parseData(data);
      if(output == ROD_OK || output == ROD_E_ROD_ERROR) {
        //Return the data
        break;
     }
     else
     {
       //parsing did not go OK, retry
       continue;
     }

    }
    //ROD_Disable();

    return output;
  }

  /************************************************************************/
  /* Private Methods                                                      */
  /************************************************************************/

  /**
   * \brief    Tries to fill _buff with a measurement.
   * \returns  ROD_OK if successful, error if not
   */
  int ROD_poll() {

    memset(_buff, 0xFF, ROD_MAX_MESSAGE_SIZE);
    HAL_UART_AbortReceive(_uart);

    if(HAL_UART_Receive(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500)==HAL_OK){ //_uart->readBlocking(&_buff[0], 22)) {
      return ROD_OK;
    } else {
      return ROD_E_ROD_NOT_FOUND;
    }
  }

  /**
   * \brief    Parses an int in _buff from the start index, until the first ,
   * \param    index     the index to start parsing at, it's updated as the
   *                     methods moves through _buff.
   * \param    val       reference to the int that has to be filled.
   * \returns  ROD_OK if successful, error if not
   */
  uint8_t ROD_parseInt(uint8_t *index, int *val) 
  {
    //resetting the value
    *val = 0;
    uint8_t neg = 0;

    while(_buff[++*index] != ',' && *index < ROD_MAX_MESSAGE_SIZE){
      
      if(_buff[*index] == '-') {
          neg = 1;
      } else if(_buff[*index] - '0' >= 0 && _buff[*index] - '0' <= 9) {
          *val *= 10;
          *val += (_buff[*index] - '0');
      } else {
        //Invalid data
        return 0;
      }
    }
    if(neg) {
      *val *= -1;
    }
    return 1;
  }

  /**
   * \brief    Parses the data in _buff and fills the given RODData struct.
   * \param    data                                 The ROD struct to fill.
   * \returns  ROD_OK if successful, error if not
   */
  int ROD_parseData(struct RODData *data) {

    // #,123,0,0,56,-704832,-952630,-929064,1083,
    // #,sensorID,impedance,phase,error,voltageA,voltageB,voltageC,temp,
    // int32,int32,int16,uint8,int32,int32,int32,int16

    uint8_t index = 0;
    uint8_t found = 0;

    //Checking for a # somewhere in the first third of the buffer
    for(index = 0;  index < ROD_MAX_MESSAGE_SIZE/3; index++) {
      if(_buff[index] == '#') {
        found = 1;
        index++;
        break;
      }
    }

    if(found == 0) {
      return ROD_E_INVALID_DATA;
    }

    //SENSOR
    if(!ROD_parseInt(&index, &data->sensorID)) {
      return ROD_E_INVALID_DATA;
    }

    //update sensor ID global var
    ROD_sensorID = &data->sensorID;

    //IMPEDANCE
    if(!ROD_parseInt(&index, &data->impedance)) {
      return ROD_E_INVALID_DATA;
    }

    //PHASE
    if(!ROD_parseInt(&index, &data->phase)) {
      return ROD_E_INVALID_DATA;
    }

    int tempErr = 0;
    //ERROR
    if(!ROD_parseInt(&index, &tempErr)) {
      return ROD_E_INVALID_DATA;
    }

    data->error = (uint8_t)tempErr;

    //VOLTAGE A
    if(!ROD_parseInt(&index, &data->voltageA)) {
      return ROD_E_INVALID_DATA;
    }

    //VOLTAGE B
    if(!ROD_parseInt(&index, &data->voltageB)) {
      return ROD_E_INVALID_DATA;
    }

    //VOLTAGE B
    if(!ROD_parseInt(&index, &data->voltageC)) {
      return ROD_E_INVALID_DATA;
    }

    //TEMPERATURE
    int temp = 0;
    if(!ROD_parseInt(&index, &data->temp)) {
      return ROD_E_INVALID_DATA;
    }

    if(data->error != 0) {
      return ROD_E_ROD_ERROR;
    }

    return ROD_OK;
  }

void ROD_Enable()
{
  #ifndef platform_octa_stm
    //set enable to low
    HAL_GPIO_WritePin(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_RESET);
  #endif
}

void ROD_Disable()
{
  #ifndef platform_octa_stm
    //set enable to high
    HAL_GPIO_WritePin(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_SET);
  #endif
}

