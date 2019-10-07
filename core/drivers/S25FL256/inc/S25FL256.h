/**
 * \file        S25FL256.hpp
 * \copyright   Copyright (c) 2017 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       S25FL256
 *
 * \details     128 Mbit (16 Mbyte)/256 Mbit (32 Mbyte) 3.0V
 *                 SPI Flash Memory
 *
 * \author      T. Boonen <thijl.boonen@imec-nl.nl>
 * \date        11-2017
 */

#include <stdint.h>
#include "stm32l4xx_hal.h"
#ifndef SENSORS_DRIVERS_S25FL256_HPP_
#define SENSORS_DRIVERS_S25FL256_HPP_

#define FLASH_BLOCK_SIZE 256 /* hardware flash size */
//TODO why not 0?
#define FIRST_FLASH_ADDRESS 0x0300000
#define LAST_FLASH_ADDRESS 0x1E84800 //30MB
#define MAX_BLOCK_NUMBER ((LAST_FLASH_ADDRESS - FIRST_FLASH_ADDRESS) / FLASH_BLOCK_SIZE) - 1

#define BAUDRATE 450000
#define DATA_WIDTH 8
//#define SPI_MODE ISPI::SpiMode::TWO

#define E_WRITE_IN_PROGRESS  -2
#define E_SPI_ERROR          -3
#define E_INVALID_BLOCK_NR   -4
#define E_LAST_BLOCK_WRITTEN -5

#define MIN_FLASH_SIZE			256


           // S25FL256();
            //~S25FL256();


            uint8_t S25FL256_Initialize(SPI_HandleTypeDef *hspi);// IGPIO* aGPIOChipSelect, IGPIO* aGPIOFlashWP, IGPIO* aGPIOFlashHOLD);
            int S25FL256_open(const uint32_t aBlockNr);

            int S25FL256_read(uint8_t *apData, const uint32_t aSize);
            int S25FL256_write(const uint8_t* apData, const uint32_t aSize);
            uint8_t S25FL256_isWriteInProgress(void);
            int S25FL256_eraseFullFlash(void);
            int S25FL256_eraseSectorFromBlock(const uint32_t aBlockNr);


            int S25FL256_flush(void);
            int S25FL256_sendCommand(const uint8_t command);
            int S25FL256_send4BCommand(const uint8_t command, const uint32_t address);
            int S25FL256_readReg(const uint8_t aReg, uint8_t aData);


            //ISPI* mSPI;
            //IGPIO* mGPIOFlashWP;
            //IGPIO* mGPIOFlashHOLD;

            uint32_t mCurrentReadAddress;
            uint32_t mCurrentWriteAddress;
            uint8_t  mMemBlock[FLASH_BLOCK_SIZE];
            uint16_t mMemBlockIndex;

            /*ISPI::SpiConfig SPIConfig = {
                SPI_MODE,
                DATA_WIDTH,
                BAUDRATE,
                nullptr //This is set in the constructor
            };*/

            uint8_t _initialised;


#endif /* SENSORS_DRIVERS_S25FL256_HPP_ */
