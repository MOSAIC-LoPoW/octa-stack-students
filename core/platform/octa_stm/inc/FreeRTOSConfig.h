#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions. *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/
/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif

#define configUSE_PREEMPTION                     1
#define configSUPPORT_STATIC_ALLOCATION          0
#define configSUPPORT_DYNAMIC_ALLOCATION         1

#if LOW_POWER==1
    #define configUSE_IDLE_HOOK                  1
#else
    #define configUSE_IDLE_HOOK                  0
#endif
#if LOW_POWER==2
    #define configUSE_TICKLESS_IDLE              2
    #define configUSE_TIMERS                     0
#else
    #define configUSE_TICKLESS_IDLE              0
    #define configUSE_TIMERS                     1
    #define configTIMER_TASK_PRIORITY            ( 3 )
    #define configTIMER_QUEUE_LENGTH             10
    #define configTIMER_TASK_STACK_DEPTH         configMINIMAL_STACK_SIZE
#endif

#define configUSE_TICK_HOOK                      0
#define configCPU_CLOCK_HZ                       ( SystemCoreClock )
#define configTICK_RATE_HZ                       ((TickType_t)1000)
#define configMAX_PRIORITIES                     ( 7 )
#define configMINIMAL_STACK_SIZE                 ((uint16_t)512)
#define configTOTAL_HEAP_SIZE                    ((size_t)49152*3)
#define configMAX_TASK_NAME_LEN                  ( 16 )
#define configUSE_16_BIT_TICKS                   0
#define configUSE_MUTEXES                        1
#define configQUEUE_REGISTRY_SIZE                8
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                    0
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 0
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_eTaskGetState                   1

#if DEBUG
    #define configUSE_TRACE_FACILITY                1
    #define configUSE_STATS_FORMATTING_FUNCTIONS    1
#endif

#if configUSE_TICKLESS_IDLE == 2
    void preSLEEP(uint32_t xModifiableIdleTime);
    void postSLEEP(uint32_t xModifiableIdleTime);

    #define configPRE_SLEEP_PROCESSING(x) (preSLEEP(x))
    #define configPOST_SLEEP_PROCESSING(x) (postSLEEP(x))

    #define configPRE_STOP_PROCESSING(x) (preSTOP(x))
    #define configPOST_STOP_PROCESSING(x) (postSTOP(x))
#endif

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
 /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 #define configPRIO_BITS         4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* use same assert callback for configASSERT */
#define configASSERT(  x  )     if( ( x ) == 0 ) assert_failed( __FILE__, __LINE__ )

#define configCHECK_FOR_STACK_OVERFLOW           2

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler

/* IMPORTANT: This define is commented when used with STM32Cube firmware, when the timebase source is SysTick,
              to prevent overwriting SysTick_Handler defined within STM32Cube HAL */
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
