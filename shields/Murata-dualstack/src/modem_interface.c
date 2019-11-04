#include <string.h>

#include "uart.h"
#include "fifo.h"
#include "modem_interface.h"
#include "errors.h"
#include "crc.h"

#define RX_BUFFER_SIZE 256

#define PLATFORM_USE_MODEM_INTERRUPT_LINES 0


#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent

UART_HandleTypeDef* uart;

uint8_t rxData[1];
static uint8_t rx_buffer[RX_BUFFER_SIZE];
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
// static pin_id_t uart_state_pin;
// static pin_id_t target_uart_state_pin;

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

bool process_rx_fifo(void *arg);
static void execute_state_machine();


/** @Brief Enable UART interface and UART interrupt
 *  @return void
 */
static void modem_interface_enable(void) 
{
  modem_listen_uart_inited = true;
}

/** @Brief disables UART interface
 *  @return void
 */
static void modem_interface_disable(void) 
{
  modem_listen_uart_inited = false;
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
  murata_release();
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
    HAL_UART_Transmit(uart, chunk, len, 1000);
    request_pending = false;
    release_receiver();
#if PLATFORM_USE_MODEM_INTERRUPT_LINES
    execute_state_machine());
#endif
  } 
  else 
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    HAL_UART_Transmit(uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE, 1000);
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
    murata_wakeup(); //set interrupt gpio to indicate ready for data
  }

  // prevent the MCU to go back to stop mode by scheduling ourself again until pin goes low,
  // to keep UART RX enabled ?
  while(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5));
}


/** @Brief Schedules flush tx fifo when receiver is ready
 *  @return void
 */
static void execute_state_machine()
{
#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  switch(state) {
    case STATE_RESP:
      // response period completed, process the request
      assert(!hw_gpio_get_in(target_uart_state_pin));
      sched_post_task(&process_rx_fifo);
      if(request_pending) {
        SWITCH_STATE(STATE_RESP_PENDING_REQ);
        sched_post_task(&execute_state_machine);
      } else {
        SWITCH_STATE(STATE_IDLE);
        hw_gpio_clr(uart_state_pin);
        sched_cancel_task(&modem_listen);
        modem_interface_disable();
      }
      break;
    case STATE_IDLE:
      if(hw_gpio_get_in(target_uart_state_pin)) {
        // wake-up requested
        SWITCH_STATE(STATE_RESP);
        modem_listen(NULL);
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
      sched_cancel_task(&modem_listen);
      SWITCH_STATE(STATE_REQ_WAIT);
      hw_gpio_set(uart_state_pin); // wake-up receiver
      DPRINT("wake-up receiver\n");
      sched_post_task(&execute_state_machine); // reschedule again to prevent sleeoping
      // in principle we could go to sleep but this will cause pin to float, this can be improved later
      break;
    case STATE_REQ_WAIT:
      if(hw_gpio_get_in(target_uart_state_pin)) {
        // receiver active
        SWITCH_STATE(STATE_REQ_BUSY);
        // fall-through to STATE_REQ_BUSY!
      } else {
        // TODO timeout
        sched_post_task(&execute_state_machine); // reschedule again to prevent sleeoping
        // in principle we could go to sleep but this will cause pin to float, this can be improved later
        break;
      }
    case STATE_REQ_BUSY:
      if(request_pending) {
        modem_interface_enable();
        sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
      } else if (!hw_gpio_get_in(target_uart_state_pin)){
        SWITCH_STATE(STATE_IDLE);
      } else
        sched_post_task(&execute_state_machine); 
        //keep active until target reacts
      break;
    case STATE_RESP_PENDING_REQ:
      assert(request_pending);
      // response period completed, initiate pending request by switching to REQ_START
      assert(!hw_gpio_get_in(target_uart_state_pin));
      hw_gpio_clr(uart_state_pin);
      SWITCH_STATE(STATE_REQ_START);
      sched_post_task(&execute_state_machine);
      break;
    default:
      DPRINT("unexpected state %i\n", state);
      assert(false);
  }
#endif
}

/** @Brief Check package counter and crc
 *  @return void
 */
static bool verify_payload(fifo_t* bytes, uint8_t* header)
{
  static uint8_t payload[RX_BUFFER_SIZE - SERIAL_FRAME_HEADER_SIZE]; // statically allocated so this does not end up on stack
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
bool process_rx_fifo(void *arg) 
{
  if(!parsed_header) 
  {
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE) 
    {
        fifo_peek(&rx_fifo, header, 0, SERIAL_FRAME_HEADER_SIZE);

        if(header[0] != SERIAL_FRAME_SYNC_BYTE || header[1] != SERIAL_FRAME_VERSION) 
        {
          fifo_skip(&rx_fifo, 1);
          //printf("skip\r\n");
          parsed_header = false;
          payload_len = 0;
          if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
            return false;
          return false;
        }
        parsed_header = true;
        fifo_skip(&rx_fifo, SERIAL_FRAME_HEADER_SIZE);
        payload_len = header[SERIAL_FRAME_SIZE];
        printf("UART RX, payload size = %i\r\n", payload_len);
        return false;
    }
  }
  else 
  {
    if(fifo_get_size(&rx_fifo) < payload_len) {
      printf("payload not complete\r\n");
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
        printf("target rebooted, reason=%i\n\r\n", reboot_reason);
        if(target_rebooted_cb)
          target_rebooted_cb(reboot_reason);
      }
      else
      {
        fifo_skip(&payload_fifo, payload_len);
        printf("!!!FRAME TYPE NOT IMPLEMENTED\r\n");
      }
      fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo)); // pop parsed bytes from original fifo
    }
    else
      printf("!!!PAYLOAD DATA INCORRECT\r\n");
    payload_len = 0;
    parsed_header = false;
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
      return false;  
    return true;
  }
  //return false;
}
/** @Brief put received UART data in fifo
 *  @return void
 */
void uart_rx_cb()
{
    HAL_UART_Receive_IT(uart, rxData, 1);
    error_t err;
    //start_atomic();
        err = fifo_put(&rx_fifo, rxData, 1); //assert(err == SUCCESS);
    //end_atomic();
    
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
    //sched_post_task(&process_rx_fifo);
    process_rx_fifo(NULL);
#endif
}

// /** @Brief Processes events on UART interrupt line
//  *  @return void
//  */
// static void uart_int_cb(void *arg)
// {
//   // do not read GPIO level here in interrupt context (GPIO clock might not be enabled yet), execute state machine instead
//   sched_post_task(&execute_state_machine);
// }

// static void modem_interface_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb) {
// #ifdef PLATFORM_USE_USB_CDC
// 	cdc_set_rx_interrupt_callback(uart_rx_cb);
// #else
//   uart_set_rx_interrupt_callback(uart, uart_rx_cb);
// #endif
// }

void modem_interface_init(UART_HandleTypeDef* uart_handle) 
{
  uart = uart_handle;
  fifo_init(&modem_interface_tx_fifo, modem_interface_tx_buffer, MODEM_INTERFACE_TX_FIFO_SIZE);
  flush_modem_interface_tx_fifo(NULL);
  execute_state_machine();
  process_rx_fifo(NULL);
  state = STATE_IDLE;
  
  // uart_state_pin=uart_state_int_pin;
  // target_uart_state_pin=target_uart_state_int_pin;
  
  fifo_init(&rx_fifo, rx_buffer, sizeof(rx_buffer));
  HAL_UART_Receive_IT(uart, rxData, 1);

// When not using interrupt lines we keep uart enabled so we can use RX IRQ.
// If the platform has interrupt lines the UART should be re-enabled when handling the modem interrupt
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
  modem_interface_enable();
#endif

  modem_interface_transfer_bytes(1, 1, SERIAL_MESSAGE_TYPE_REBOOTED);
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
   
  //start_atomic();
  request_pending = true;
  fifo_put(&modem_interface_tx_fifo, (uint8_t*) &header, SERIAL_FRAME_HEADER_SIZE);
  fifo_put(&modem_interface_tx_fifo, bytes, length);
  //end_atomic();

#if PLATFORM_USE_MODEM_INTERRUPT_LINES
  execute_state_machine();
#else
  flush_modem_interface_tx_fifo(NULL); // state machine is not used when not using interrupt lines
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
