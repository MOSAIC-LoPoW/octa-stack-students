#include "platform.h"
#include "inttypes.h"

static void (*DeInit_Application_Function)(void) = 0;
static void (*ReInit_Application_Function)(void) = 0;

void Initialize_Platform(void)
{
    //GPIO, IWDG, USB_UART(Logging)
    OCTA_Initialize_Common_Peripherals();
    //P1 UART, I2C, SPI & GPIO handles
    OCTA_Initialize_P1_Peripherals();
    //P2 UART, I2C, SPI & GPIO handles
    OCTA_Initialize_P2_Peripherals();
}

void OCTA_Initialize_Common_Peripherals(void)
{
    #if USE_BOOTLOADER
        bootloader_SetVTOR();
    #endif
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();

    OCTA_GPIO_Init();
    OCTA_IWDG_Init();
    FLASH_SPI_Init();
    common_I2C_Init();
    RTC_Init();
    USB_UART_Init(115200);

    // Get Unique ID of octa
    short_UID = get_UID();
    octa_uid.u64 = short_UID;

    // Print Welcome Message
    printWelcome();
}

void OCTA_ReInitialize_Common_Peripherals(void)
{
    #if USE_BOOTLOADER
        bootloader_SetVTOR();
    #endif
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();

    OCTA_GPIO_Init();
    FLASH_SPI_Init();
    common_I2C_Init();
    USB_UART_Init(115200);
}

void OCTA_DeInitialize_Common_Peripherals()
{
    HAL_UART_DeInit(&USB_UART);
    HAL_I2C_DeInit(&common_I2C);
    HAL_SPI_DeInit(&FLASH_SPI);
    OCTA_GPIO_DeInit();
}

void OCTA_Initialize_P1_Peripherals(void)
{
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
}

void OCTA_Initialize_P2_Peripherals(void)
{
    P2_header.number = 2;
    P2_header.uartHandle = &P2_UART;
    P2_header.i2cHandle = &P2_I2C;
    P2_header.spiHandle = &P2_SPI;
    P2_header.DIO1 = &P2_DIO1;
    P2_header.DIO2 = &P2_DIO2;
    P2_header.DIO3 = &P2_DIO3;
    P2_header.DIO4 = &P2_DIO4;
    P2_header.DIO5 = &P2_DIO5;
    P2_header.DIO6 = &P2_DIO6;
}

uint64_t get_UID(void)
{
    return (*((uint64_t *)(UID_BASE)) << 32) + *((uint64_t *)(UID_BASE + 0x04U));
}

void platform_initialize_UART(struct OCTA_header aHeader, uint32_t aBaudRate)
{
    switch(aHeader.number)
    {
        case 1:
            P1_UART_Init(aBaudRate);
            break;
        case 2:
            P2_UART_Init(aBaudRate);
            break;
        default:
            break;
    }
}

void platform_initialize_I2C(struct OCTA_header aHeader)
{
    switch(aHeader.number)
    {
        case 1:
            P1_I2C_Init();
            break;
        case 2:
            P2_I2C_Init();
            break;
        default:
            break;
    }
}

void platform_initialize_SPI(struct OCTA_header aHeader)
{
    switch(aHeader.number)
    {
        case 1:
            P1_SPI_Init();
            break;
        case 2:
            P2_SPI_Init();
            break;
        default:
            break;
    }
}

struct OCTA_header platform_getHeader(uint8_t connector)
{
    switch (connector)
    {
    case 1:
        P1_header.active = 1;
        return P1_header;
    case 2:
        P2_header.active = 1;
        return P2_header;
    default:
        //invalid connector
        printERR("Invalid connector\r\n");
        struct OCTA_header invalid; 
        invalid.active = 0;
        return invalid;
    }
}

char* ullx(uint64_t val)
{
    static char buf[34] = { [0 ... 33] = 0 };
    char* out = &buf[33];
    uint64_t hval = val;
    unsigned int hbase = 10;

    do {
        *out = "0123456789"[hval % hbase];
        --out;
        hval /= hbase;
    } while(hval);
    
    *out-- = ' ', *out = ' ';

    return out;
}

__weak void printWelcome(void)
{
    printf("\r\n***************************************************\r\n");
    //APP SETTINGS
    #ifdef APPLICATION_NAME && APPLICATION_VERSION
        printf("Application: %s v%s\r\n", STR(APPLICATION_NAME),STR(APPLICATION_VERSION)); 
        sscanf(STR(APPLICATION_VERSION), "%d.%d.%d", &appversion_major, &appversion_minor, &appversion_patch);
    #else
        printf("unknown application name & version\r\n");
    #endif
    #ifdef STACK_VERSION
        printf("Stack version: %s\r\n", STR(STACK_VERSION));
        sscanf(STR(STACK_VERSION), "%d.%d.%d", &stackversion_major, &stackversion_minor, &stackversion_patch);
    #else
        printf("unknown stack version\r\n");
    #endif
    #ifdef platform_octa
        printf("Platform: octa + nucleo\r\n");
    #elif platform_octa_stm
        printf("Platform: octa_stm\r\n");
    #endif
    printf("Settings: ");
    #if USE_RTOS_SCHEDULER
        printf("scheduler");
    #else
        printf("noscheduler");
    #endif
    #if USE_BOOTLOADER
        printf(", bootlader");
    #endif
    #if DEBUG
        printf(", debug");
    #endif
    #ifdef LOW_POWER
        printf(", lowpower mode %d", LOW_POWER);
    #endif
    #if Disable_STEPUP
        printf(", 5V-stepup");
    #endif
    #if USE_FTDI_LOGGING
        printf(", ftdi-logging");
    #endif
    #if PRINT_FLOATS
        printf(", print-floats");
    #endif
    printf("\r\n");

    //OCTA UID
    char UIDString[sizeof(short_UID)];
    memcpy(UIDString, &short_UID, sizeof(short_UID));
    printf("octa uid (u64 dec):%s \r\n", ullx(short_UID));
    printf("octa uid (u64 le hex): ");
    for (const char* p = UIDString; *p; ++p)
        {
            printf("%02x", *p);
        }

    //SHIELDS
    printf("\r\n\r\n");
    #ifdef FIREFLY_CONNECTOR
        printf("*   GPS-Firefly at connector P%d, v%s\r\n", FIREFLY_CONNECTOR, STR(FIREFLY_VERSION));
    #endif
    #ifdef MURATA_ONLYLORAWAN
        printf("*   Murata LoRaWAN at connector P%d using %s keys, v%s\r\n", MURATA_CONNECTOR, STR(LORAWAN_APPLICATIONNAME), STR(MURATA_VERSION));
    #endif
    #ifdef MURATA_DUALSTACK
        printf("*   Murata Dualstack at connector P%d using %s LoRaWAN keys, v%s\r\n", MURATA_CONNECTOR, STR(LORAWAN_APPLICATIONNAME), STR(MURATA_VERSION));
    #endif
    #ifdef NB_IOT_CHIP_SARAN
        #ifdef NB_IOT_CONNECTOR
            printf("*   SARAN NB-IoT at connector P%d using provider %s, v%s\r\n", NB_IOT_CONNECTOR, STR(NB_IOT_PROVIDERNAME), STR(NB_IOT_VERSION));
            #ifdef SARAN21X_Params_Exists
                printf("        -SARAN21X_Params file found\r\n");
            #else
                printf("        -SARAN21X_Params file not found, using example params file\r\n");
            #endif
        #endif
    #endif
    #ifdef NB_IOT_CHIP_NORDIC
        #ifdef NB_IOT_CONNECTOR
            printf("*   Nordic NB-IoT at connector P%d using provider %s, v%s\r\n", NB_IOT_CONNECTOR, STR(NB_IOT_PROVIDERNAME), STR(NB_IOT_VERSION));
        #else
            printf("*   Nordic NB-IoT (onboard) using provider %s, v%s\r\n", STR(NB_IOT_PROVIDERNAME), STR(NB_IOT_VERSION));
        #endif
        #ifdef NORDIC9160_Params_Exists
            printf("        -NORDIC9160_Params file found\r\n");
        #else
            printf("        -NORDIC9160_Params file not found, using example params file\r\n");
        #endif
    #endif
    #ifdef NOVA_PM_CONNECTOR
        printf("*   Nova Particle Sensor at connector P%d, v%s\r\n", NOVA_PM_CONNECTOR, STR(NOVA_PM_VERSION));
    #endif        
    #ifdef ROD_CONNECTOR
        printf("*   ROD at connector P%d, v%s\r\n", ROD_CONNECTOR, STR(ROD_VERSION));
    #endif
    #ifdef SIGFOX_CONNECTOR
        printf("*   SigFox at connector P%d, v%s\r\n", SIGFOX_CONNECTOR, STR(SIGFOX_VERSION));
    #endif
    printf("\r\n***************************************************\r\n\r\n");

    #if DEBUG && USE_RTOS_SCHEDULER && PRINT_RTOS_STATS
        osThreadDef(rtos_stats, RTOS_Print_Stats_Thread, osPriorityNormal, 0, 512);
        rtosprintHandle = osThreadCreate(osThread(rtos_stats), NULL);
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = OCTA_BTN2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        //enable the interrupt
        HAL_NVIC_SetPriority(EXTI1_IRQn, 8, 0);		
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);
        printDBG("***************************************************\r\n");
        printDBG("DEBUG Enabled, press BTN2 to get RTOS Thread stats\r\n");
        printDBG("***************************************************\r\n\r\n");
    #endif
    
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_Delay(200);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_Delay(200);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    HAL_Delay(200);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
    HAL_Delay(200);
    HAL_GPIO_TogglePin(OCTA_RLED_GPIO_Port, OCTA_RLED_Pin);
    HAL_GPIO_TogglePin(OCTA_GLED_GPIO_Port, OCTA_GLED_Pin);
    HAL_GPIO_TogglePin(OCTA_BLED_GPIO_Port, OCTA_BLED_Pin);
}

void vApplicationIdleHook(void){
  #if LOW_POWER==1
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
  #endif
}

void DeInitialize_Platform(void)
{
    OCTA_DeInitialize_Common_Peripherals();

    //app specific deinit
    if(DeInit_Application_Function)
    {
        DeInit_Application_Function();
    }
}

void ReInitialize_Platform(void)
{
    OCTA_ReInitialize_Common_Peripherals();
    
    //app specific reinit
    if(ReInit_Application_Function)
    {
        ReInit_Application_Function();
    }
}

void set_Application_DeInit_Function(void (*func))
{
    DeInit_Application_Function = func;
}
void set_Application_ReInit_Function(void (*func))
{
    ReInit_Application_Function = func;
}