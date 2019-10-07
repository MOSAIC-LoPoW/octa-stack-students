#include <string.h>

//#include "hwuart.h"

//#include "framework_defs.h"
//#include "platform_defs.h"
#include "fifo.h"
//#include "scheduler.h"
#include "modem_interface.h"
//#include "hal_defs.h"
//#include "debug.h"
#include "errors.h"
//#include "platform_defs.h"

//#include "hwsystem.h"
//#include "hwatomic.h"
#include "crc.h"

//#include "ng.h"
#include "crc.h"

//#include "log.h"
#include "stm32l4xx_hal.h"


#define RX_BUFFER_SIZE 256

#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent
#define PLATFORM_USE_MODEM_INTERRUPT_LINES 0
#define HAL_UART_USE_DMA_TX 1
//static uart_handle_t* uart;
UART_HandleTypeDef* uart_handle;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rxData[1];
fifo_t rx_fifo;

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_INTERFACE_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINT_DATA(...)
#endif


#define SERIAL_FRAME_SYNC_BYTE 0xC0
#define SERIAL_FRAME_VERSION   0x00
#define SERIAL_FRAME_HEADER_SIZE 7
#define SERIAL_FRAME_SIZE 4
#define SERIAL_FRAME_COUNTER 2
#define SERIAL_FRAME_TYPE 3
#define SERIAL_FRAME_CRC1   5
#define SERIAL_FRAME_CRC2   6

#define MODEM_INTERFACE_TX_FIFO_SIZE 255
static uint8_t modem_interface_tx_buffer[MODEM_INTERFACE_TX_FIFO_SIZE];
static fifo_t modem_interface_tx_fifo;
static bool request_pending = false;

uint8_t header[SERIAL_FRAME_HEADER_SIZE];
static uint8_t payload_len = 0;
static uint8_t packet_up_counter = 0;
static uint8_t packet_down_counter = 0;
//static pin_id_t uart_state_pin;
//static pin_id_t target_uart_state_pin;

static bool modem_listen_uart_inited = false;
static bool parsed_header = false;

static cmd_handler_t alp_handler;
static cmd_handler_t ping_response_handler;
static cmd_handler_t logging_handler;
static target_rebooted_callback_t target_rebooted_cb;

typedef enum {
  STATE_IDLE,
  STATE_REQ_START,
  STATE_REQ_WAIT,
  STATE_REQ_BUSY,
  STATE_RESP,
  STATE_RESP_PENDING_REQ
} state_t;

static state_t state = STATE_IDLE;

#define SWITCH_STATE(s) do { \
  state = s; \
  DPRINT("switch to %s\n", #s); \
} while(0)

bool process_rx_fifo(void);
void execute_state_machine(void);


/** @Brief Enable UART interface and UART interrupt
 *  @return void
 */
static void modem_interface_enable(void) 
{
  DPRINT("uart enabled @ %i",timer_get_counter_value());
  //assert(uart_enable(uart));
  //uart_rx_interrupt_enable(uart);
  modem_listen_uart_inited = true;
}

/** @Brief disables UART interface
 *  @return void
 */
static void modem_interface_disable(void) 
{
  modem_listen_uart_inited = false;
  //assert(uart_disable(uart));
  DPRINT("uart disabled @ %i",timer_get_counter_value());
}

/** @brief Lets receiver know that 
 *  all the data has been transfered
 *  @return void
 */
static void release_receiver()
{
#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  DPRINT("release receiver\n");
  modem_interface_disable();
  //hw_gpio_clr(uart_state_pin);
  modem_release();
#endif
}

/** @brief transmit data in fifo to UART
 *  @return void
 */
static void flush_modem_interface_tx_fifo(void *arg) 
{
  uint8_t len = fifo_get_size(&modem_interface_tx_fifo);

#ifdef HAL_UART_USE_DMA_TX
  // when using DMA we transmit the whole FIFO at once
  uint8_t buffer[MODEM_INTERFACE_TX_FIFO_SIZE];
  fifo_pop(&modem_interface_tx_fifo, buffer, len);
  //uart_send_bytes(uart, buffer, len);
  HAL_UART_Transmit(uart_handle, buffer, len, 1000);
#else
  // only send small chunks over uart each invocation, to make sure
  // we don't interfer with critical stack timings.
  // When there is still data left in the fifo this will be rescheduled
  // with lowest prio
  uint8_t chunk[TX_FIFO_FLUSH_CHUNK_SIZE];
  if(len <= TX_FIFO_FLUSH_CHUNK_SIZE)
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, len);
    //uart_send_bytes(uart, chunk, len);
    HAL_UART_Transmit(uart_handle, chunk, len, 1000);
    request_pending = false;
    release_receiver();
#if PLATFORM_USE_MODEM_INTERRUPT_LINES
    //sched_post_task(&execute_state_machine);
    execute_state_machine();
#endif
  } 
  else 
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    //uart_send_bytes(uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
      HAL_UART_Transmit(uart_handle, chunk, TX_FIFO_FLUSH_CHUNK_SIZE, 1000);
    //sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
    flush_modem_interface_tx_fifo(NULL);
  }
#endif
}

/** @Brief Keeps µC awake while receiving UART data
 *  @return void
 */
static void modem_listen(void* arg)
{
  if(!modem_listen_uart_inited)
  {
    modem_interface_enable();
    //hw_gpio_set(uart_state_pin); //set interrupt gpio to indicate ready for data
    modem_wakeup();
    DPRINT("MODEM_ENABLE")
  }
  // prevent the MCU to go back to stop mode by scheduling ourself again until pin goes low,
  // to keep UART RX enabled
  //sched_post_task_prio(&modem_listen, MIN_PRIORITY, NULL);
  while(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5));
}


/** @Brief Schedules flush tx fifo when receiver is ready
 *  @return void
 */
void execute_state_machine(void)
{
#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  switch(state) {
    case STATE_RESP:
      // response period completed, process the request
      //assert(!hw_gpio_get_in(target_uart_state_pin));
      //sched_post_task(&process_rx_fifo);
      process_rx_fifo();
      if(request_pending) {
        SWITCH_STATE(STATE_RESP_PENDING_REQ);
        //sched_post_task(&execute_state_machine);
        execute_state_machine();
      } else {
        SWITCH_STATE(STATE_IDLE);
        //hw_gpio_clr(uart_state_pin);
        modem_release();
        //sched_cancel_task(&modem_listen);??
        modem_interface_disable();
      }
      break;
    case STATE_IDLE:
      //if(hw_gpio_get_in(target_uart_state_pin)) {
      if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5)){
        // wake-up requested
        SWITCH_STATE(STATE_RESP);
        modem_listen(NULL);

        execute_state_machine();

        break;
      } else if(request_pending) { //check if we are really requesting a start
        SWITCH_STATE(STATE_REQ_START);
        // fall-through to STATE_REQ_START!
      } else
      {
        break;
      }
    case STATE_REQ_START:
      // TODO timeout
      //sched_cancel_task(&modem_listen);
      SWITCH_STATE(STATE_REQ_WAIT);
      //hw_gpio_set(uart_state_pin); // wake-up receiver
      modem_wakeup();
      DPRINT("wake-up receiver\n");
      //sched_post_task(&execute_state_machine); // reschedule again to prevent sleeoping
      execute_state_machine();
      // in principle we could go to sleep but this will cause pin to float, this can be improved later
      break;
    case STATE_REQ_WAIT:
      //if(hw_gpio_get_in(target_uart_state_pin)) {
      if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5)){
        // receiver active
        SWITCH_STATE(STATE_REQ_BUSY);
        //execute_state_machine();
        // fall-through to STATE_REQ_BUSY!
      } else {
        // TODO timeout
        //sched_post_task(&execute_state_machine); // reschedule again to prevent sleeoping
        execute_state_machine();
        // in principle we could go to sleep but this will cause pin to float, this can be improved later
        //SWITCH_STATE(STATE_REQ_BUSY);
        break;
      }
    case STATE_REQ_BUSY:
      if(request_pending) {
        modem_interface_enable();
        //sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
        flush_modem_interface_tx_fifo(NULL);
      //} else if (!hw_gpio_get_in(target_uart_state_pin)){
      }else if (!HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5)){
        SWITCH_STATE(STATE_IDLE);
      } else{
        //sched_post_task(&execute_state_machine); 
        execute_state_machine();
        //keep active until target reacts
      }
      break;
    case STATE_RESP_PENDING_REQ:
      //assert(request_pending);
      // response period completed, initiate pending request by switching to REQ_START
      //assert(!hw_gpio_get_in(target_uart_state_pin));
      //hw_gpio_clr(uart_state_pin);
      modem_release();
      SWITCH_STATE(STATE_REQ_START);
      //sched_post_task(&execute_state_machine);
      execute_state_machine();
      break;
    default:
      DPRINT("unexpected state %i\n", state);
      //assert(false);
  }
#endif
}

/** @Brief Check package counter and crc
 *  @return void
 */
static bool verify_payload(fifo_t* bytes, uint8_t* header)
{
  uint8_t payload[header[SERIAL_FRAME_SIZE]];
  fifo_peek(bytes, (uint8_t*) &payload, 0, header[SERIAL_FRAME_SIZE]);

  //check for missing packages
  packet_down_counter++;
  if(header[SERIAL_FRAME_COUNTER]!=packet_down_counter)
  {
    //TODO consequence? (save total missing packages?)
    //log_print_string("!!! missed packages: %i",(header[SERIAL_FRAME_COUNTER]-packet_down_counter));
    packet_down_counter=header[SERIAL_FRAME_COUNTER]; //reset package counter
  }

  DPRINT("RX HEADER: ");
  DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("RX PAYLOAD: ");
  DPRINT_DATA(payload, header[SERIAL_FRAME_SIZE]);

  uint16_t calculated_crc = crc_calculate(payload, header[SERIAL_FRAME_SIZE]);
 
  if(header[SERIAL_FRAME_CRC1]!=((calculated_crc >> 8) & 0x00FF) || header[SERIAL_FRAME_CRC2]!=(calculated_crc & 0x00FF))
  {
    //TODO consequence? (request repeat?)
    //log_print_string("CRC incorrect!");
    return false;
  }
  else
    return true;
}

/** @Brief Processes received uart data
 * 1) Search for sync bytes (always)
 * 2) get header size and parse header
 * 3) Wait for correct # of bytes (length present in header)
 * 4) Execute crc check and check message counter
 * 5) send to corresponding service (alp, ping service, log service)
 *  @return void
 */
bool process_rx_fifo(void) 
{
  if(!parsed_header) 
  {
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE) 
    {
        fifo_peek(&rx_fifo, header, 0, SERIAL_FRAME_HEADER_SIZE);

        if(header[0] != SERIAL_FRAME_SYNC_BYTE || header[1] != SERIAL_FRAME_VERSION) 
        {
          fifo_skip(&rx_fifo, 1);
          DPRINT("skip");
          parsed_header = false;
          payload_len = 0;
          if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
            //process_rx_fifo();
            //sched_post_task(&process_rx_fifo);
          return false;
        }
        parsed_header = true;
        fifo_skip(&rx_fifo, SERIAL_FRAME_HEADER_SIZE);
        payload_len = header[SERIAL_FRAME_SIZE];
        DPRINT("UART RX, payload size = %i", payload_len);
        //sched_post_task(&process_rx_fifo);
        //process_rx_fifo();
    }
  }
  else 
  {
    if(fifo_get_size(&rx_fifo) < payload_len) {
      //process_rx_fifo();
      return false;
    }
    // payload complete, start parsing
    // rx_fifo can be bigger than the current serial packet, init a subview fifo
    // which is restricted to payload_len so we can't parse past this packet.
    fifo_t payload_fifo;
    fifo_init_subview(&payload_fifo, &rx_fifo, 0, payload_len);
  
    if(verify_payload(&payload_fifo,(uint8_t *)&header))
    {
      if(header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_ALP_DATA && alp_handler != NULL)
        alp_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_RESPONSE  && ping_response_handler != NULL)
        ping_response_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_LOGGING && logging_handler != NULL)
        logging_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_REQUEST)
      {
        uint8_t ping_reply[1]={0x02};
        fifo_skip(&payload_fifo,1);
        modem_interface_transfer_bytes((uint8_t*) &ping_reply,1,SERIAL_MESSAGE_TYPE_PING_RESPONSE);
      }
      else if(header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_REBOOTED)
      {
        uint8_t reboot_reason;
        fifo_pop(&payload_fifo, &reboot_reason, 1);
        printf("target rebooted, reason=%i\n", reboot_reason);
        if(target_rebooted_cb)
          target_rebooted_cb(reboot_reason);
      }
      else
      {
        fifo_skip(&payload_fifo, payload_len);
        DPRINT("!!!FRAME TYPE NOT IMPLEMENTED");
      }
      fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo)); // pop parsed bytes from original fifo
    }
    else
      DPRINT("!!!PAYLOAD DATA INCORRECT");
    payload_len = 0;
    parsed_header = false;
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
        //process_rx_fifo();
      //sched_post_task(&process_rx_fifo);
    return true;
  }
  return false;
}
/** @Brief put received UART data in fifo
 *  @return void
 */
static void uart_rx_cb(uint8_t data)
{
    error_t err;
    //start_atomic();
        err = fifo_put(&rx_fifo, &data, 1); assert(err == SUCCESS);
    //end_atomic();

#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
    //sched_post_task(&process_rx_fifo);
    process_rx_fifo();
#endif
}

void modem_interface_uart_rx_cb()
{
    HAL_UART_Receive_IT(uart_handle, rxData, 1);
    error_t err;
    //start_atomic();
        err = fifo_put(&rx_fifo, rxData, 1); //assert(err == SUCCESS);
    //end_atomic();
    
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
    //sched_post_task(&process_rx_fifo);
    process_rx_fifo();
#endif
}

/** @Brief Processes events on UART interrupt line
 *  @return void
 */
// static void uart_int_cb(pin_id_t pin_id, uint8_t event_mask)
// {
//   // do not read GPIO level here in interrupt context (GPIO clock might not be enabled yet), execute state machine instead
//   //sched_post_task(&execute_state_machine);
// }

// static void modem_interface_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb) {
// #ifdef PLATFORM_USE_USB_CDC
// 	cdc_set_rx_interrupt_callback(uart_rx_cb);
// #else
//   uart_set_rx_interrupt_callback(uart, uart_rx_cb);
// #endif
// }

//void modem_interface_init(uint8_t idx, uint32_t baudrate, pin_id_t uart_state_int_pin, pin_id_t target_uart_state_int_pin) 
void modem_interface_init(UART_HandleTypeDef* uart)
{
  uart_handle = uart;
  fifo_init(&modem_interface_tx_fifo, modem_interface_tx_buffer, MODEM_INTERFACE_TX_FIFO_SIZE);
  //sched_register_task(&flush_modem_interface_tx_fifo);
  flush_modem_interface_tx_fifo(NULL);
  //sched_register_task(&execute_state_machine);
  execute_state_machine();
  //sched_register_task(&process_rx_fifo);
  process_rx_fifo();
  state = STATE_IDLE;
  //uart_state_pin=uart_state_int_pin;
  //target_uart_state_pin=target_uart_state_int_pin;

  //uart = uart_init(idx, baudrate,0);
  
  
  fifo_init(&rx_fifo, rx_buffer, sizeof(rx_buffer));
  //modem_interface_set_rx_interrupt_callback(&uart_rx_cb);

  HAL_UART_Receive_IT(uart_handle, rxData, 1);

#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  //assert(sched_register_task(&modem_listen) == SUCCESS);
  DPRINT("using interrupt lines");
  //assert(hw_gpio_configure_interrupt(target_uart_state_pin, &uart_int_cb, GPIO_RISING_EDGE | GPIO_FALLING_EDGE) == SUCCESS);
  //assert(hw_gpio_enable_interrupt(target_uart_state_pin) == SUCCESS);
  //if(hw_gpio_get_in(target_uart_state_pin))
  if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5))
  {
    //sched_post_task(&modem_listen);
    modem_listen(NULL);
    DPRINT("Ready to receive (boot)");// @ %i",timer_get_counter_value());
  }
#endif

// When not using interrupt lines we keep uart enabled so we can use RX IRQ.
// If the platform has interrupt lines the UART should be re-enabled when handling the modem interrupt
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
  modem_interface_enable();
#endif

  //uint8_t reboot_reason = (uint8_t)hw_system_reboot_reason();
  //modem_interface_transfer_bytes(&reboot_reason, 1, SERIAL_MESSAGE_TYPE_REBOOTED);
}

void modem_interface_transfer_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type) 
{
  uint8_t header[SERIAL_FRAME_HEADER_SIZE];
  uint16_t crc=crc_calculate(bytes,length);

  packet_up_counter++;
  header[0] = SERIAL_FRAME_SYNC_BYTE;
  header[1] = SERIAL_FRAME_VERSION;

  header[SERIAL_FRAME_COUNTER] = packet_up_counter;
  header[SERIAL_FRAME_TYPE] = type;
  header[SERIAL_FRAME_SIZE] = length;
  header[SERIAL_FRAME_CRC1] = (crc >> 8) & 0x00FF;
  header[SERIAL_FRAME_CRC2] = crc & 0x00FF;

  DPRINT("TX HEADER:");
  DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("TX PAYLOAD:");
  DPRINT_DATA(bytes, length);
   
//  start_atomic();
  request_pending = true;
  fifo_put(&modem_interface_tx_fifo, (uint8_t*) &header, SERIAL_FRAME_HEADER_SIZE);
  fifo_put(&modem_interface_tx_fifo, bytes, length);
 // end_atomic();

#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  //sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, NULL);
    execute_state_machine();
#else
  //sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL); // state machine is not used when not using interrupt lines
    flush_modem_interface_tx_fifo(NULL);
#endif  
}

void modem_interface_transfer(char* string) {
  modem_interface_transfer_bytes((uint8_t*) string, strnlen(string, 100), SERIAL_MESSAGE_TYPE_LOGGING); 
}


void modem_interface_register_handler(cmd_handler_t cmd_handler, serial_message_type_t type)
{
  if(type == SERIAL_MESSAGE_TYPE_ALP_DATA) 
    alp_handler=cmd_handler;
  else if(type == SERIAL_MESSAGE_TYPE_PING_RESPONSE) 
    ping_response_handler=cmd_handler;
  else if(type == SERIAL_MESSAGE_TYPE_LOGGING) 
    logging_handler=cmd_handler;
  else
    DPRINT("Modem interface callback not implemented");
}

void modem_interface_set_target_rebooted_callback(target_rebooted_callback_t cb)
{
  target_rebooted_cb = cb;
}
