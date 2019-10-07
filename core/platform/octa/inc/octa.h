#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "sysclock.h"
#include "rtosmacros.h"
#include "gpio.h"
#include "iwdg.h"
#include "uart.h"
#include "i2c.h"
#include "spi.h"
#if USE_BOOTLOADER
    #include "bootloader.h"
#endif

#define UID_BASE 0x1FFF7590

struct OCTA_header{
    uint8_t                     number;
    UART_HandleTypeDef          *uartHandle;
    I2C_HandleTypeDef           *i2cHandle;
    SPI_HandleTypeDef           *spiHandle;
    struct OCTA_GPIO            *DIO1;
    struct OCTA_GPIO            *DIO2;
    struct OCTA_GPIO            *DIO3;
    struct OCTA_GPIO            *DIO4;
    struct OCTA_GPIO            *DIO5;
    struct OCTA_GPIO            *DIO6;
    uint8_t                     active;
};

struct OCTA_header P1_header;
struct OCTA_header P2_header;
struct OCTA_header P3_header;

void OCTA_Initialize_Platform(void);
void OCTA_Initialize_Common_Peripherals(void);
void OCTA_Initialize_P1_Peripherals(void);
void OCTA_Initialize_P2_Peripherals(void);
void OCTA_Initialize_P3_Peripherals(void);
uint64_t get_UID(void);
void platform_initialize_UART(struct OCTA_header aHeader, uint32_t aBaudRate);
void platform_initialize_I2C(struct OCTA_header aHeader);
void platform_initialize_SPI(struct OCTA_header aHeader);
struct OCTA_header platform_getHeader(uint8_t connector);