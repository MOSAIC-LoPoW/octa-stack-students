/**
 * \file        TD1207.hpp
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       TD1207
 *
 * \details     Driver for TD1207.
 *
 * \author      J.Bergs johan.bergs@uantwerpen.be, B.Dil <bram.dil@imec-nl.nl>
 * \date        07-2018
 */

#ifndef TD1207_HPP_
#define TD1207_HPP_

#include "stm32l4xx_hal.h"

#define TD1207_BAUDRATE 9600
#define TD1207_BUFFER_SIZE 128

/*
		 * Initializes the sensor using its default configuration
		 */
uint8_t TD1207_Initialize(void);
void TD1207_toggleResetPin(void);
uint8_t TD1207_send(uint8_t *bytes, uint8_t length, uint8_t ack);

#endif //TD1207_HPP_
