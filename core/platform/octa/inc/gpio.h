/* Includes ------------------------------------------------------------------*/
#include "stm32l496xx.h"
#include "stm32l4xx_hal.h"

struct OCTA_GPIO{
    GPIO_TypeDef* PORT;
    uint16_t    PIN;
};

// OCTA P1
#define P1_DIO1_Pin             GPIO_PIN_9
#define P1_DIO1_GPIO_Port       GPIOC
struct OCTA_GPIO P1_DIO1;
#define P1_DIO2_Pin             GPIO_PIN_8
#define P1_DIO2_GPIO_Port       GPIOC
struct OCTA_GPIO P1_DIO2;
#define P1_DIO3_Pin             GPIO_PIN_3
#define P1_DIO3_GPIO_Port       GPIOF
struct OCTA_GPIO P1_DIO3;
#define P1_DIO4_Pin             GPIO_PIN_7
#define P1_DIO4_GPIO_Port       GPIOA
struct OCTA_GPIO P1_DIO4;
#define P1_DIO5_Pin             GPIO_PIN_5
#define P1_DIO5_GPIO_Port       GPIOF
struct OCTA_GPIO P1_DIO5;
#define P1_DIO6_Pin             GPIO_PIN_3
#define P1_DIO6_GPIO_Port       GPIOC
struct OCTA_GPIO P1_DIO6;

// OCTA P2
#define P2_DIO1_Pin             GPIO_PIN_2
#define P2_DIO1_GPIO_Port       GPIOE
struct OCTA_GPIO P2_DIO1;
#define P2_DIO2_Pin             GPIO_PIN_4
#define P2_DIO2_GPIO_Port       GPIOE
struct OCTA_GPIO P2_DIO2;
#define P2_DIO3_Pin             GPIO_PIN_6
#define P2_DIO3_GPIO_Port       GPIOE
struct OCTA_GPIO P2_DIO3;
#define P2_DIO4_Pin             GPIO_PIN_5
#define P2_DIO4_GPIO_Port       GPIOE
struct OCTA_GPIO P2_DIO4;
#define P2_DIO5_Pin             GPIO_PIN_8
#define P2_DIO5_GPIO_Port       GPIOF
struct OCTA_GPIO P2_DIO5;
#define P2_DIO6_Pin             GPIO_PIN_3
#define P2_DIO6_GPIO_Port       GPIOE
struct OCTA_GPIO P2_DIO6;

// OCTA P3
#define P3_DIO1_Pin             GPIO_PIN_7
#define P3_DIO1_GPIO_Port       GPIOE
struct OCTA_GPIO P3_DIO1;
#define P3_DIO2_Pin             GPIO_PIN_10
#define P3_DIO2_GPIO_Port       GPIOE
struct OCTA_GPIO P3_DIO2;
#define P3_DIO3_Pin             GPIO_PIN_14
#define P3_DIO3_GPIO_Port       GPIOF
struct OCTA_GPIO P3_DIO3;
#define P3_DIO4_Pin             GPIO_PIN_8
#define P3_DIO4_GPIO_Port       GPIOE
struct OCTA_GPIO P3_DIO4;
#define P3_DIO5_Pin             GPIO_PIN_9
#define P3_DIO5_GPIO_Port       GPIOE
struct OCTA_GPIO P3_DIO5;
#define P3_DIO6_Pin             GPIO_PIN_1
#define P3_DIO6_GPIO_Port       GPIOB
struct OCTA_GPIO P3_DIO6;

// OCTA GPIO
#define OCTA_STEPUP_Pin             GPIO_PIN_9
#define OCTA_STEPUP_GPIO_Port       GPIOF
#define OCTA_BTN1_Pin               GPIO_PIN_0
#define OCTA_BTN1_GPIO_Port         GPIOG
#define OCTA_BTN2_Pin               GPIO_PIN_1
#define OCTA_BTN2_GPIO_Port         GPIOG
#define OCTA_GLED_Pin               GPIO_PIN_0
#define OCTA_GLED_GPIO_Port         GPIOB
#define OCTA_RLED_Pin               GPIO_PIN_14
#define OCTA_RLED_GPIO_Port         GPIOD
#define OCTA_BLED_Pin               GPIO_PIN_15
#define OCTA_BLED_GPIO_Port         GPIOD
#define OCTA_LIGHTSENSORLED_Pin     GPIO_PIN_7
#define OCTA_LIGHTSENSORLED_Port    GPIOF
#define OCTA_FLASH_CS_Pin           GPIO_PIN_13
#define OCTA_FLASH_CS_Port          GPIOF
#define OCTA_FLASH_HOLD_Pin         GPIO_PIN_0
#define OCTA_FLASH_HOLD_Port        GPIOE  
#define OCTA_FLASH_WP_Pin           GPIO_PIN_11
#define OCTA_FLASH_WP_Port          GPIOE      

// OCTA BLE GPIO
#define BLE_Reset_Pin               GPIO_PIN_1
#define BLE_Reset_GPIO_Port         GPIOA

// NUCLEO GPIO
#define B1_Pin                      GPIO_PIN_13
#define B1_GPIO_Port                GPIOC
#define LD2_Pin                     GPIO_PIN_7
#define LD2_GPIO_Port               GPIOB
#define LD3_Pin                     GPIO_PIN_14
#define LD3_GPIO_Port               GPIOB

void OCTA_GPIO_Init(void);


