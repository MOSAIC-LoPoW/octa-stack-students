/**
 * \file        NORDIC9160.h
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       NORDIC9160
 *
 * \details     Driver for NORDIC9160 NB-IoT module.
 *
 * \author      Mahfoudhi Farouk <farouk.mahfoudhi@uantwerpen.be>
 * \date        08-2018
 */
#ifndef NORDIC9160_HPP_
#define NORDIC9160_HPP_

//use the NORDIC9160_Params_Example file to create your own
#ifdef NORDIC9160_Params_Exists
    #include "NORDIC9160_Params.h"
#else
    #include "NORDIC9160_Params_example.h"
#endif
#include "stm32l4xx.h"
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NORDIC9160_BAUDRATE 115200
#define NORDIC9160_BUFFER_SIZE 512
#define NORDIC9160_MAX_RX_BUFFER_SIZE 1300
#define NORDIC9160_READ_UART_ATTEMPTS 10
#define NORDIC9160_WAIT_FOR_REGISTERING_ATTEMPTS 100
#define NORDIC9160_LOCAL_SEND_PORT 1234

enum module_registration_status
{
    NB_IOT_ERROR,
    NOT_REGISTERED,
    REGISTERED,
    REGISTERING
};

union {
    uint64_t u64;
    struct
    {
        uint8_t b1, b2, b3, b4, b5, b6, b7, b8;
    } bytes;
} imsi_number;

union {
    uint32_t u32;
    struct
    {
        uint8_t b1, b2, b3, b4;
    } bytes;
} cell_id_struct;

union {
    int32_t int32;
    struct
    {
        uint8_t b1, b2, b3, b4;
    } bytes;
} tx_power_struct;

bool NORDIC9160_Initialize(void);
void NORDIC9160_toggleResetPin(void);
bool NORDIC9160_init_module(const char *server_ip, const uint32_t server_port);
bool NORDIC9160_send(uint8_t *msg, uint16_t length);
bool NORDIC9160_send_to_server(uint8_t *msg, uint16_t length, const char *server_ip, const uint32_t server_port);
bool NORDIC9160_get_imsi(char *imsi);
uint64_t NORDIC9160_convert_imsi_char_to_u64(const char* text);
uint8_t NORDIC9160_get_registration_status(void);
bool NORDIC9160_wait_for_registering(void);
bool NORDIC9160_get_ip(char *ip);
bool NORDIC9160_get_cell_id(char *cell);
bool NORDIC9160_get_nuestats(char *cell, char *txpower);
bool NORDIC9160_get_RSSI(char *RSSI);
void NORDIC9160_setApplicationRxCallback(void (*callback)(uint8_t *buffer, uint16_t len));
void NORDIC9160_rxCallback(void);
void NORDIC9160_set_server_parameters(const char* server_ip, const uint32_t server_port);

uint16_t NORDIC9160_read_line(char *ptr, uint16_t len, bool debug, uint32_t delay);
void NORDIC9160_Disable_Interrupt(void);
bool NORDIC9160_reboot(void); // bind()
bool NORDIC9160_turn_on(void);
bool NORDIC9160_set_low_power_mode(void);
bool NORDIC9160_set_pdp_context(void);
bool NORDIC9160_set_band(void);
bool NORDIC9160_select_plmn(void);
bool NORDIC9160_UDP_disconnect(void);
bool NORDIC9160_UDP_connect(void);
bool NORDIC9160_autoconnect(void);
bool SARAN22X_autoconnect(void);
bool bindsocket(uint8_t *socket);
bool NORDIC9160_open_socket(uint8_t *socket);
bool NORDIC9160_close_socket(uint8_t socket);
void NORDIC9160_handleRxData(void);

#endif //NORDIC9160_HPP_
