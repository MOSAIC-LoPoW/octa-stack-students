#include "unity.h"
#include "mock_help_stm32l4xx_hal_uart.h"
#include "mock_stm32l4xx_hal_gpio.h"
#include "mock_help_platform.h"
#include "mock_iwdg.h"
#include "mock_stm32l4xx_hal.h"
#include "mock_help_uart.h"
#include "print.h"
#include "NORDIC9160.h"

extern UART_HandleTypeDef *uartHandle;
extern struct OCTA_header NB_IoTHeader;
extern struct OCTA_header P1_header;
extern struct OCTA_GPIO *nb_iot_reset_pin_nordic;
extern bool nordic_hw_initialized;
extern bool nordic_initialized;

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
    nordic_hw_initialized = false;
    nordic_initialized = false;
    P1_header.active = 1;
    NB_IoTHeader = P1_header;
    nb_iot_reset_pin_nordic = NB_IoTHeader.DIO2;
    uartHandle = NB_IoTHeader.uartHandle;
    IWDG_feed_Ignore();
    HAL_Delay_Ignore();
}

// every test file requires this function;
// tearDown() is called by the generated runner before each test case function
void tearDown(void) 
{
} 

void test_NORDIC9160_Initialize_Should_ReturnFalseIfAlreadyInitialized(void)
{
    //set initialised to true -> already initialised
    nordic_hw_initialized = true;
    TEST_ASSERT_EQUAL(NORDIC9160_Initialize(), false);
}

void test_NORDIC9160_Initialize_Should_ReturnFalseIfInvalidConnectorIsProvided(void)
{
    //p1_header.active = 0 simulates invalid header
    P1_header.active = 0;
    platform_getHeader_ExpectAndReturn(1, P1_header);
    TEST_ASSERT_EQUAL(NORDIC9160_Initialize(), false);
}

void test_NORDIC9160_Initialize_Should_Return1AfterSuccessfullInit(void)
{
    platform_getHeader_ExpectAndReturn(1, P1_header);
    platform_initialize_UART_Expect(NB_IoTHeader, NORDIC9160_BAUDRATE);
    UART_SetShieldCallback_Expect(&NORDIC9160_rxCallback, 1);
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_SET);

    TEST_ASSERT_EQUAL(NORDIC9160_Initialize(), true);
}

void test_NORDIC9160_Initialize_Should_Return0WhenCallingInitAgainAfterSuccessfullInit(void)
{
    platform_getHeader_ExpectAndReturn(1, P1_header);
    platform_initialize_UART_Expect(NB_IoTHeader, NORDIC9160_BAUDRATE);
    UART_SetShieldCallback_Expect(&NORDIC9160_rxCallback, 1);
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_SET);
    
    TEST_ASSERT_EQUAL(NORDIC9160_Initialize(), true);

    TEST_ASSERT_EQUAL(NORDIC9160_Initialize(), false);
}

void test_NORDIC9160_Should_ToggleResetPin(void)
{
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin_Expect(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_SET);
    NORDIC9160_toggleResetPin();
}