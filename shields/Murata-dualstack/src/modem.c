/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "modem.h"
//#include "debug.h"
//#include "log.h"
#include "errors.h"
#include "fifo.h"
#include "alp.h"
//#include "scheduler.h"
//#include "timer.h"
#include "d7ap.h"
#include <stdio.h>
#include <stdlib.h>
//#include "platform.h"
#include "modem_interface.h"
#include "stm32l4xx_hal.h"

#define RX_BUFFER_SIZE 256
#define CMD_BUFFER_SIZE 256

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif


typedef struct {
  uint8_t tag_id;
  bool is_active;
  fifo_t fifo;
  uint8_t buffer[256];
} command_t;

static modem_callbacks_t* callbacks;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static command_t command; // TODO only one active command supported for now
static uint8_t next_tag_id = 0;
static bool parsed_header = false;
static uint8_t payload_len = 0;
extern fifo_t rx_fifo;

static void process_serial_frame(fifo_t* fifo) {
  bool command_completed = false;
  bool completed_with_error = false;
  while(fifo_get_size(fifo)) {
    alp_action_t action;
    alp_parse_action(fifo, &action);

    switch(action.operation) {
      case ALP_OP_RETURN_TAG:
        if(action.tag_response.tag_id == command.tag_id) {
          command_completed = action.tag_response.completed;
          completed_with_error = action.tag_response.error;
        } else {
          printf("received resp with unexpected tag_id (%i vs %i)", action.tag_response.tag_id, command.tag_id);
          // TODO unsolicited responses
        }
        break;
      case ALP_OP_WRITE_FILE_DATA:
        if(callbacks->write_file_data_callback)
          callbacks->write_file_data_callback(action.file_data_operand.file_offset.file_id,
                                               action.file_data_operand.file_offset.offset,
                                               action.file_data_operand.provided_data_length,
                                               action.file_data_operand.data);
        break;
      case ALP_OP_RETURN_FILE_DATA:
        if(callbacks->return_file_data_callback)
          callbacks->return_file_data_callback(action.file_data_operand.file_offset.file_id,
                                               action.file_data_operand.file_offset.offset,
                                               action.file_data_operand.provided_data_length,
                                               action.file_data_operand.data);
        break;
      case ALP_OP_RETURN_STATUS:
        if(action.status.type==ALP_ITF_ID_D7ASP)
        {
          d7ap_session_result_t interface_status = *((d7ap_session_result_t*)action.status.data);
          uint8_t addressee_len = d7ap_addressee_id_length(interface_status.addressee.ctrl.id_type);
          //printf("received resp from: ");
          //printf(interface_status.addressee.id, addressee_len);
        }
        if(callbacks->modem_interface_status_callback)
          callbacks->modem_interface_status_callback(action.status.type,&action.status.data);
        break;
      default:
        printf("unknown serial frame\r\n");
        //assert(false);
    }
  }


  if(command_completed) {
    DPRINT("LoRaWAN modem command with tag %i completed ", command.tag_id);
    if(callbacks->command_completed_callback)
      callbacks->command_completed_callback(completed_with_error,command.tag_id);

    command.is_active = false;
  }

}

void modem_cb_init(modem_callbacks_t* cbs)
{
    command.is_active = false;
    modem_interface_register_handler(&process_serial_frame, SERIAL_MESSAGE_TYPE_ALP_DATA);
    callbacks = cbs;
    if(cbs->modem_rebooted_callback)
      modem_interface_set_target_rebooted_callback(cbs->modem_rebooted_callback);
}

uint8_t modem_process_fifo()
{
  uint8_t retries = 0;
  uint8_t status = 0;
  while(fifo_get_size(&rx_fifo) && retries < 50)
  { 
    status = (uint8_t)process_rx_fifo(NULL);
    HAL_Delay(50);
    retries++;
  }
  return status;
}

void modem_reinit() {
  command.is_active = false;
}

void modem_send_ping() {
  uint8_t ping_request[1]={0x01};
  modem_interface_transfer_bytes((uint8_t*) &ping_request, 1, SERIAL_MESSAGE_TYPE_PING_REQUEST);
}

bool modem_execute_raw_alp(uint8_t* alp, uint8_t len) {
  modem_interface_transfer_bytes(alp, len, SERIAL_MESSAGE_TYPE_ALP_DATA);
}

bool alloc_command() {
  if(command.is_active) {
    printINF("prev command still active\r\n");
    return false;
  }

  command.is_active = true;
  fifo_init(&command.fifo, command.buffer, CMD_BUFFER_SIZE);
  command.tag_id = next_tag_id;
  next_tag_id++;

  alp_append_tag_request_action(&command.fifo, command.tag_id, true);
  return true;
}

bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size) {
  if(!alloc_command())
    return false;

  alp_append_read_file_data_action(&command.fifo, file_id, offset, size, true, false);

  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);

  return true;
}

bool modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data) {
  if(!alloc_command())
    return false;

  alp_append_write_file_data_action(&command.fifo, file_id, offset, size, data, true, false);

  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);

  return true;
}

bool modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data,
                                     session_config_t* session_config) {
  if(!alloc_command())
    return false;

  if(session_config->interface_type==DASH7)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_D7ASP, (uint8_t *) &session_config->d7ap_session_config, sizeof(d7ap_session_config_t));
  else if(session_config->interface_type==LORAWAN_OTAA)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_LORAWAN_OTAA, (uint8_t *) &session_config->lorawan_session_config_otaa, sizeof(lorawan_session_config_otaa_t));
  else if(session_config->interface_type==LORAWAN_ABP)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_LORAWAN_ABP, (uint8_t *) &session_config->lorawan_session_config_abp, sizeof(lorawan_session_config_abp_t));

  alp_append_return_file_data_action(&command.fifo, file_id, offset, length, data);

  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);
  return true;
}

bool modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length,
                                         session_config_t* session_config) {
  if(!alloc_command())
    return false;

   if(session_config->interface_type==DASH7)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_D7ASP, (uint8_t *) &session_config->d7ap_session_config, sizeof(d7ap_session_config_t));
  else if(session_config->interface_type==LORAWAN_OTAA)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_LORAWAN_OTAA, (uint8_t *) &session_config->lorawan_session_config_otaa, sizeof(lorawan_session_config_otaa_t));
  else if(session_config->interface_type==LORAWAN_ABP)
    alp_append_forward_action(&command.fifo, ALP_ITF_ID_LORAWAN_ABP, (uint8_t *) &session_config->lorawan_session_config_abp, sizeof(lorawan_session_config_abp_t));

  fifo_put(&command.fifo, alp_command, length);

  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);
  return true;
}

uint8_t modem_get_active_tag_id()
{
  return next_tag_id;
}
