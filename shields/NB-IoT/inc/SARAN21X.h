/**
 * \file        SARAN21X.h
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       SARAN21X
 *
 * \details     Driver for SARAN210 NB-IoT module.
 *
 * \author      Mahfoudhi Farouk <farouk.mahfoudhi@uantwerpen.be>
 * \date        08-2018
 */
#ifndef SARAN21X_HPP_
#define SARAN21X_HPP_

//use the SARAN21X_Params_Example file to create your own
#ifdef SARAN21X_Params_Exists
    #include "SARAN21X_Params.h"
#else
    #include "SARAN21X_Params_example.h"
#endif
#include "stm32l4xx.h"
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SARAN210_BAUDRATE 9600
#define SARAN210_BUFFER_SIZE 512
#define SARAN210_MAX_RX_BUFFER_SIZE 1300
#define SARAN210_READ_UART_ATTEMPTS 10
#define SARAN210_WAIT_FOR_REGISTERING_ATTEMPTS 100
#define SARAN210_LOCAL_SEND_PORT 1234

struct OCTA_GPIO *nb_iot_reset_pin;

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

bool SARAN21X_Initialize(void);
void SARAN21X_toggleResetPin(void);
bool SARAN21X_init_module(const char *server_ip, const uint32_t server_port);
bool SARAN21X_send(uint8_t *msg, uint16_t length);
bool SARAN21X_send_to_server(uint8_t *msg, uint16_t length, const char *server_ip, const uint32_t server_port);
bool SARAN21X_receive(uint16_t length);
bool SARAN21X_get_imsi(char *imsi);
uint64_t SARAN21X_convert_imsi_char_to_u64(const char* text);
uint8_t SARAN21X_get_registration_status(void);
bool SARAN21X_wait_for_registering(void);
bool SARAN21X_get_ip(char *ip);
bool SARAN21X_get_cell_id(char *cell);
bool SARAN21X_get_nuestats(char *cell, char *txpower);
bool SARAN21X_get_RSSI(char *RSSI);
void SARAN21X_setApplicationRxCallback(void (*callback)(uint8_t *buffer, uint16_t len));
void SARAN21X_rxCallback(void);
void SARAN21X_set_server_parameters(const char* server_ip, const uint32_t server_port);

uint16_t SARAN21X_read_line(char *ptr, uint16_t len, bool debug, uint32_t delay);
void SARAN21X_Disable_Interrupt(void);
bool SARAN21X_reboot(void);
bool SARAN21X_turn_on(void);
bool SARAN21X_set_low_power_mode(void);
bool SARAN21X_set_pdp_context(void);
bool SARAN21X_set_band(void);
bool SARAN21X_select_plmn(void);
bool SARAN21X_autoconnect(void);
bool SARAN21X_open_socket(uint8_t *socket);
bool SARAN21X_close_socket(uint8_t socket);
void SARAN21X_handleRxData(void);

#endif //SARAN21X_HPP_
