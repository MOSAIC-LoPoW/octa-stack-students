#include "unity.h"
#include "mock_help_stm32l4xx_hal_uart.h"
#include "mock_stm32l4xx_hal_gpio.h"
#include "mock_help_platform.h"
#include "mock_iwdg.h"
#include "print.h"
#include "ROD.h"
#include <math.h>

#define ROD_MAX_MESSAGE_SIZE 100
#define ROD_MAX_RETRIES 5
#define ROD_MAX_MESSAGE_SIZE 100
#define ROD_IMP_KCELL 0.8
#define ROD_MAX_PHASE 15

extern uint8_t ROD_initialised;
extern struct OCTA_header RODHeader;
extern struct OCTA_header P1_header;
extern struct OCTA_GPIO *ROD_enable_pin;
extern UART_HandleTypeDef *_uart;
extern uint8_t _buff[ROD_MAX_MESSAGE_SIZE];

struct RODData testRODData;

uint8_t ind = 0;

void setUp(void) 
{
    //platform init code for connector
    P1_header.number = 1;
    P1_header.uartHandle = &P1_UART;
    P1_header.i2cHandle = &P1_I2C;
    P1_header.spiHandle = &P1_SPI;
    P1_header.DIO1 = &P1_DIO1;
    P1_header.DIO2 = &P1_DIO2;
    P1_header.DIO3 = &P1_DIO3;
    P1_header.DIO4 = &P1_DIO4;
    P1_header.DIO5 = &P1_DIO5;
    P1_header.DIO6 = &P1_DIO6;

    // set some variables (done in init)
    ROD_initialised = 0;
    P1_header.active = 1;
    RODHeader = P1_header;
    ROD_enable_pin = P1_header.DIO1;
    _uart = RODHeader.uartHandle;
    
    IWDG_feed_Ignore();
}

// every test file requires this function;
// tearDown() is called by the generated runner before each test case function
void tearDown(void) 
{
} 

void test_ROD_Initialize_Should_Return0IfAlreadyInitialized(void)
{
    //set initialised to 1 -> already initialised
    ROD_initialised = 1;
    TEST_ASSERT_EQUAL(ROD_Initialize(), 0);
}

void test_ROD_Initialize_Should_Return1AfterSuccessfullInit(void)
{
    platform_getHeader_ExpectAndReturn(1, P1_header);
    platform_initialize_UART_Expect(RODHeader, ROD_BAUDRATE);
    HAL_GPIO_WritePin_Expect(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_SET);
    
    TEST_ASSERT_EQUAL(ROD_Initialize(), 1);
}

void test_ROD_Initialize_Should_Return0WhenCallingInitAgainAfterSuccessfullInit(void)
{
    platform_getHeader_ExpectAndReturn(1, P1_header);
    platform_initialize_UART_Expect(RODHeader, ROD_BAUDRATE);
    HAL_GPIO_WritePin_Expect(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_SET);
    
    TEST_ASSERT_EQUAL(ROD_Initialize(), 1);

    TEST_ASSERT_EQUAL(ROD_Initialize(), 0);
}

void test_ROD_Enable_Should_Reset_ROD_Enable_Pin(void)
{
    HAL_GPIO_WritePin_Expect(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_RESET);
    ROD_Enable();
}

void test_ROD_Disable_Should_Set_ROD_Enable_Pin(void)
{
    HAL_GPIO_WritePin_Expect(ROD_enable_pin->PORT, ROD_enable_pin->PIN, GPIO_PIN_SET);
    ROD_Disable();
}

void test_ROD_poll_Should_Return_E_ROD_NOT_FOUND_whenRODNotFound(void)
{
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_TIMEOUT);
    TEST_ASSERT_EQUAL(ROD_poll(), ROD_E_ROD_NOT_FOUND);
}

void test_ROD_poll_Should_Return_ROD_OK_WhenPollingOK(void)
{
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    TEST_ASSERT_EQUAL(ROD_poll(), ROD_OK);
}

void test_ROD_parseInt_Should_return_0_OnInvalid_Data(void)
{
    char invalid_data[ROD_MAX_MESSAGE_SIZE] = "#fdsakj,123456,";
    memcpy(_buff, invalid_data, ROD_MAX_MESSAGE_SIZE);
    ind = 0;
    TEST_ASSERT_EQUAL(ROD_parseInt(&ind, &testRODData.sensorID), 0);

    memset(invalid_data, 0, ROD_MAX_MESSAGE_SIZE);
    invalid_data[ROD_MAX_MESSAGE_SIZE] = "#dsaffasdfasdf";
    memcpy(_buff, invalid_data, ROD_MAX_MESSAGE_SIZE);
    ind = 0;
    TEST_ASSERT_EQUAL(ROD_parseInt(&ind, &testRODData.sensorID), 0);
}

void test_ROD_parseInt_Should_return_1_OnValid_Data(void)
{
    char valid_data[ROD_MAX_MESSAGE_SIZE] = ",1234356,0,0,56,-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    ind = 0;
    TEST_ASSERT_EQUAL(ROD_parseInt(&ind, &testRODData.sensorID), 1);

    //negative
    valid_data[ROD_MAX_MESSAGE_SIZE] = ",-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    ind = 0;
    TEST_ASSERT_EQUAL(ROD_parseInt(&ind, &testRODData.impedance), 1);
}

void test_ROD_parseData_should_return_E_INVALID_DATA_WhenDataIsInvalid(void)
{
    uint8_t invalid_data[ROD_MAX_MESSAGE_SIZE] = "jdsfhakjhdsfafhjasdhjfgasdfjkhgasdf#,123,0,0,56";
    memcpy(_buff, invalid_data, ROD_MAX_MESSAGE_SIZE);
    TEST_ASSERT_EQUAL(ROD_parseData(&testRODData), ROD_E_INVALID_DATA);
}

void test_ROD_parseData_should_return_ROD_OK_WhenDataIsValid(void)
{
    char valid_data[ROD_MAX_MESSAGE_SIZE] = "#,123,0,0,0,-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    TEST_ASSERT_EQUAL(ROD_parseData(&testRODData), ROD_OK);

    //junk in first 1/3 of message should still be parsed
    valid_data[ROD_MAX_MESSAGE_SIZE] = "jdsfhakjhdsf#,123,0,0,0,-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    TEST_ASSERT_EQUAL(ROD_parseData(&testRODData), ROD_OK);
}

void test_ROD_parseData_should_return_E_ROD_ERROR_WhenDataIsValidButErrorFieldIsNotZero(void)
{
    char valid_data[ROD_MAX_MESSAGE_SIZE] = "#,123,0,0,1,-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    TEST_ASSERT_EQUAL(ROD_parseData(&testRODData), ROD_E_ROD_ERROR);

    //junk in first 1/3 of message should still be parsed
    valid_data[ROD_MAX_MESSAGE_SIZE] = "jdsfhakjhdsf#,123,0,0,2,-704832,-952630,-929064,1083,";
    memcpy(_buff, valid_data, ROD_MAX_MESSAGE_SIZE);
    TEST_ASSERT_EQUAL(ROD_parseData(&testRODData), ROD_E_ROD_ERROR);
}

void test_ROD_getData_should_return_E_ROD_NOT_FOUND_WhenRODNotFound_For_ROD_MAX_RETRIES(void)
{
    for(uint8_t i=0; i<ROD_MAX_RETRIES;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_TIMEOUT);
    }
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_E_ROD_NOT_FOUND);
}

void test_ROD_getData_should_return_ROD_OK_When_PollOK_AfterLessThan_ROD_MAX_RETRIES_Fails(void)
{
    //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_TIMEOUT);
    }

    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    HAL_UART_Receive_ReturnArrayThruPtr_pData("#,123,0,0,0,-704832,-952630,-929064,1083,", 100);

    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_OK);
}

void test_ROD_getData_should_return_ROD_E_ROD_ERROR_When_PollOK_ButErrorNotZero_AfterLessThan_ROD_MAX_RETRIES_Fails(void)
{
    // //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_TIMEOUT);
    }

    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK); 
    //error field not zero
    HAL_UART_Receive_ReturnArrayThruPtr_pData("#,123,0,0,12,-704832,-952630,-929064,1083,", 100);
 
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_E_ROD_ERROR);
}


void test_ROD_getData_should_return_ROD_E_INVALID_DATA_When_PollOK_ButInvalidData_AfterLessThan_ROD_MAX_RETRIES_Fails(void)
{
    //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_TIMEOUT);
    }
    
    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    //invalid data
    HAL_UART_Receive_ReturnArrayThruPtr_pData("askdjhfasdlkfhalskjdhfakljsdhfaklsjdhf,", 100);
    
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_E_INVALID_DATA);
}

void test_ROD_getData_should_return_ROD_OK_OnParsingSuccessfullyAfterFailingFirst(void)
{
     //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
        //invalid data
        HAL_UART_Receive_ReturnArrayThruPtr_pData("askdjhfasdlkfhalskjdhfakljsdhfaklsjdhf,", 100);
    }
    
    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    //valid data without error data
    HAL_UART_Receive_ReturnArrayThruPtr_pData("#,123,0,0,0,-704832,-952630,-929064,1083,", 100);
    
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_OK);
}

void test_ROD_getData_should_return_ROD_E_ERROR_OnParsingSuccessfullyWithErrorNonZeroAfterFailingFirst(void)
{
     //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
        //invalid data
        HAL_UART_Receive_ReturnArrayThruPtr_pData("askdjhfasdlkfhalskjdhfakljsdhfaklsjdhf,", 100);
    }
    
    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    //valid data without error data
    HAL_UART_Receive_ReturnArrayThruPtr_pData("#,123,0,0,13,-704832,-952630,-929064,1083,", 100);
    
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_E_ROD_ERROR);
}

void test_ROD_getData_should_return_ROD_OK_OnParsingDataStartingWithInvalidBytesSuccessfullyAfterFailingFirst(void)
{
     //fail ROD_MAX_RETRIES-1
    for(uint8_t i=0; i<ROD_MAX_RETRIES-1;i++)
    {
        HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
        HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
        //invalid data
        HAL_UART_Receive_ReturnArrayThruPtr_pData("askdjhfasdlkfhalskjdhfakljsdhfaklsjdhf,", 100);
    }
    
    //success final time
    HAL_UART_AbortReceive_ExpectAndReturn(_uart, HAL_OK);
    HAL_UART_Receive_ExpectAndReturn(_uart, &_buff[0], ROD_MAX_MESSAGE_SIZE, 1500, HAL_OK);
    //valid data without error data, starting with some invalid bytes
    HAL_UART_Receive_ReturnArrayThruPtr_pData("askdjhfasd#,123,0,0,0,-704832,-952630,-929064,1083,", 100);
    
    TEST_ASSERT_EQUAL(ROD_getData(&testRODData), ROD_OK);
}