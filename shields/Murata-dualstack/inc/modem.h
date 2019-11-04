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

#ifndef __MODEM_H__
#define __MODEM_H__

#include "d7ap.h"
//#include "hwuart.h"
#include "lorawan_stack.h"
#include "alp.h"
#include "modem_interface.h"

// TODO for now we are assuming running on OSS-7, we can refactor later
// so it is more portable

typedef void (*modem_command_completed_callback_t)(bool with_error,uint8_t tag_id);
typedef void (*modem_return_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);
typedef void (*modem_write_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);
typedef void (*modem_interface_status_callback_t)(alp_itf_id_t interface_type, uint8_t* data);

typedef struct {
    modem_command_completed_callback_t command_completed_callback;
    modem_interface_status_callback_t modem_interface_status_callback;
    modem_return_file_data_callback_t return_file_data_callback;
    modem_write_file_data_callback_t write_file_data_callback;
    target_rebooted_callback_t modem_rebooted_callback;
} modem_callbacks_t;

void modem_init(UART_HandleTypeDef* uart);
//void modem_init();
void modem_send_ping();
void modem_cb_init(modem_callbacks_t* cbs);
void modem_reinit();
bool modem_execute_raw_alp(uint8_t* alp, uint8_t len);
bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size);
bool modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data);
bool modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, session_config_t* session_config);
bool modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length, session_config_t* session_config);
uint8_t modem_get_active_tag_id();

#endif
