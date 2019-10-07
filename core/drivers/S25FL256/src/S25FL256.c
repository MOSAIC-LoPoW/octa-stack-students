/**
 * \file        S25FL256.c
 *
 * \details     128 Mbit (16 Mbyte)/256 Mbit (32 Mbyte) 3.0V
 *                 SPI Flash Memory
 *
 * \author      Mats De Meyer 
 * \date        09-2018
 */
#include "S25FL256.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "string.h" // for memcpy.

#define SPI_WRDI_CMD 0x04 // Write disable
#define SPI_RDSR_CMD 0x05 // Read Status Register
#define SPI_WREN_CMD 0x06 // Write enable

#define SPI_PP_4B_CMD 0x12   //Page Program (4-byte address). Enables writing
#define SPI_READ_4B_CMD 0x13 //Read (4-byte Address)

#define SPI_BE_CMD 0x60 //Bulk erase
#define SPI_SE_CMD 0xDC //Bulk erase

#define WRITE_IN_PROGRESS_MASK 0x01 //Bit 0 of RDSR is "Write in Progress"

#define SPI_WAIT 50

//One because it's only used to read a register
uint8_t rxData[1];
//Five, because the max amount of bytes is 5, in send4BCommand
uint8_t txData[5];

SPI_HandleTypeDef *flashSPI;

/**
    * \brief   Initializes the S25FL256 driver.
    * \returns 0 if init is successful
    * \returns E_INIT_FAILED if SPI could not be configured.
    */
uint8_t S25FL256_Initialize(SPI_HandleTypeDef *hspi)
{

    mMemBlockIndex = 0;
    memset(mMemBlock, 0, FLASH_BLOCK_SIZE);
    mCurrentReadAddress = FIRST_FLASH_ADDRESS;
    mCurrentWriteAddress = FIRST_FLASH_ADDRESS;
    _initialised = 0;

    flashSPI = hspi;
    //set CS pin to High
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

    //disable HOLD & WP
    //set WP pin high
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
    //set HOLD pin high
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
    
    _initialised = 1;

    return _initialised;
}

/**
    * \brief    Sets the read and write address to the start of this block. The max value is defined in MAX_BLOCK_NUMBER.
    * \param    aBlockNr      The 256 byte block you want to start reading or writing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_BLOCK_NR if aBlockNr is invalid.
    */
int S25FL256_open(const uint32_t aBlockNr)
{
    HAL_Delay(SPI_WAIT);

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (aBlockNr > MAX_BLOCK_NUMBER)
    {
        return E_INVALID_BLOCK_NR;
    }

    uint32_t newAddress = FIRST_FLASH_ADDRESS + aBlockNr * (FLASH_BLOCK_SIZE);
    mCurrentReadAddress = newAddress;
    mCurrentWriteAddress = newAddress;

    return 0;
}

/**
    * \brief    Reads the given number of bytes from flash.
    * \param    apData     Pointer to the uint8_t array to fill.
    * \param    aSize      The number of bytes to be read.
    * \returns  the number of bytes read if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_read(uint8_t *apData, const uint32_t aSize)
{

    HAL_Delay(SPI_WAIT);

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    //Starting the read.
    if (S25FL256_send4BCommand(SPI_READ_4B_CMD, mCurrentReadAddress) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    HAL_SPI_Receive(flashSPI, apData, (uint16_t)aSize, HAL_MAX_DELAY);

    //__HAL_SPI_DISABLE(flashSPI);
    S25FL256_Disable();

    return aSize;
}

/**
    * \brief    Buffers the given array. Once 256 bytes are cached, the data is flushed to flash.
    * \param    apData     Pointer to the uint8_t array with data to write.
    * \param    aSize      The number of bytes to be written.
    * \returns  The number of written bytes.
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    * \returns  E_LAST_BLOCK_WRITTEN if the last block was written and no more data can be buffered.
    */
int S25FL256_write(const uint8_t *apData, const uint32_t aSize)
{

    HAL_Delay(SPI_WAIT);

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (mCurrentWriteAddress > LAST_FLASH_ADDRESS)
    {
        return E_LAST_BLOCK_WRITTEN;
    }

    uint32_t bytesLeftToWrite = aSize;

    while (mMemBlockIndex + bytesLeftToWrite >= FLASH_BLOCK_SIZE)
    {

        //The number of bytes we will add to mMemBlock in this loop
        uint32_t bytesToWrite = FLASH_BLOCK_SIZE - mMemBlockIndex;

        //Filling the remainder of the buffer
        memcpy(&mMemBlock[mMemBlockIndex], apData, FLASH_BLOCK_SIZE);

        int flushedBytes = S25FL256_flush();

        //And writing the block of FLASH_BLOCK_SIZE bytes. This also clears mMemBlock and mMemBlockIndex.
        if (flushedBytes < 0)
        {
            //Flush failed
            return flushedBytes;
        }

        //Moving the pointer
        apData += bytesToWrite;
        bytesLeftToWrite -= bytesToWrite;
    }

    if (mCurrentWriteAddress > LAST_FLASH_ADDRESS)
    {
        //Flash is full, no more buffering.
        return E_LAST_BLOCK_WRITTEN;
    }
    else if (bytesLeftToWrite > 0)
    {
        //We still have some bytes left to cache.
        memcpy(&mMemBlock[mMemBlockIndex], apData, bytesLeftToWrite);
        bytesLeftToWrite = 0;
    }

    return aSize - bytesLeftToWrite;
}

/**
    * \brief    Returns the state of the write in process status bit.
    * \returns  true is the flash is performing a write action.
    */
uint8_t S25FL256_isWriteInProgress(void)
{
    HAL_Delay(SPI_WAIT);
    uint8_t status;
    if (S25FL256_readReg(SPI_RDSR_CMD, status) == 0)
    {
        return (uint8_t)(status & WRITE_IN_PROGRESS_MASK);
    }

    return 0;
}

/**
    * \brief    Erases the entire flash.
                This method is a write action. Poll with isWriteInProgress()
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_eraseFullFlash(void)
{

    HAL_Delay(SPI_WAIT);

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;        
    }

    //Setting write enable
    if (S25FL256_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual bulk erase
    if (S25FL256_sendCommand(SPI_BE_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //And disabling write again.
    if (S25FL256_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/**
    * \brief    Erases 256kb of data, starting from the given block.
                This method is a write action. Poll with isWriteInProgress()
    * \param    aBlockNr   The block you want to start erasing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_BLOCK_NR if aBlockNr is invalid.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_eraseSectorFromBlock(const uint32_t aBlockNr)
{

    HAL_Delay(SPI_WAIT);

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (aBlockNr > MAX_BLOCK_NUMBER)
    {
        return E_INVALID_BLOCK_NR;
    }

    //Setting write enable
    if (S25FL256_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual sector erase
    if (S25FL256_send4BCommand(SPI_SE_CMD, (FIRST_FLASH_ADDRESS + aBlockNr * (FLASH_BLOCK_SIZE))) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    S25FL256_Disable();

    //Disabling write.
    if (S25FL256_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/************************************************************************/
/* Private Methods                                                       */
/************************************************************************/

/**
    * \brief    Reads the value of a register.
    * \param    aReg      The register ytou want to read
    * \param    aData     Pointer to the data location.
    * \returns  0 if successful
    * \returns E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_readReg(uint8_t aReg, uint8_t aData)
{
    txData[0] = aReg;
    aData = 0x00;

    __HAL_SPI_ENABLE(flashSPI);
    S25FL256_Enable();

    HAL_SPI_Transmit(flashSPI, (const uint8_t *)txData, 1, HAL_MAX_DELAY);

    HAL_SPI_Receive(flashSPI, (const uint8_t *)rxData, 1, HAL_MAX_DELAY);

    __HAL_SPI_DISABLE(flashSPI);
    S25FL256_Disable();
    aData = rxData[0];

    return 0;
}

/**
    * \brief   Flushes the internal buffer and writes it to flash.
       This method should only be called when mMemBlock is full.
    * \returns 0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_flush(void)
{

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    //Setting write enable
    if (S25FL256_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    // write command and address adress
    if (S25FL256_send4BCommand(SPI_PP_4B_CMD, mCurrentWriteAddress) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    HAL_SPI_Transmit(flashSPI, (const uint8_t *)mMemBlock, FLASH_BLOCK_SIZE, HAL_MAX_DELAY);

    //__HAL_SPI_DISABLE(flashSPI);
    S25FL256_Disable();

    //Setting write disable
    if (S25FL256_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //It's possible that we pass LAST_FLASH_ADDRESS here, but the write method checks for that.
    mCurrentWriteAddress += FLASH_BLOCK_SIZE;

    // Clearing mMemBlock and mMemBlockIndex
    memset(mMemBlock, 0, FLASH_BLOCK_SIZE);
    mMemBlockIndex = 0;

    return 0;
}

/**
    * \brief    Sends a single command to the flash
    * \param    command The command to be sent.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_sendCommand(const uint8_t command)
{

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    txData[0] = command;

    __HAL_SPI_ENABLE(flashSPI);
    S25FL256_Enable();

    HAL_SPI_Transmit(flashSPI, (const uint8_t *)txData, 1, HAL_MAX_DELAY);

    __HAL_SPI_DISABLE(flashSPI);
    S25FL256_Disable();

    return 0;
}

/**
    * \brief    Sends a 4 byte address command
    * \param    command   The command to be sent.
    * \param    aBlockNr  The 4 byte address
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int S25FL256_send4BCommand(const uint8_t command, const uint32_t address)
{

    if (S25FL256_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    uint8_t index = 0;
    txData[index++] = command;

    //Writing 4 byte address.
    txData[index++] = (uint8_t)((address >> 24) & 0x000000FF);
    txData[index++] = (uint8_t)((address >> 16) & 0x000000FF);
    txData[index++] = (uint8_t)((address >> 8) & 0x000000FF);
    txData[index++] = (uint8_t)(address & 0x000000FF);

    //We have to enable SPI here, because isWriteInProgress disables it.
    //We don't disable it here, because CS has to stay high after this command.
    __HAL_SPI_ENABLE(flashSPI);
    S25FL256_Enable();

    //send the command over SPI
    HAL_SPI_Transmit(flashSPI, (const uint8_t *)txData, index, HAL_MAX_DELAY);

    return 0;
}

/* Manually toggle enable pin, due to it not being a default SPI pin */
void S25FL256_Enable()
{
    //set CS pin to low
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
}

/* Manually toggle enable pin, due to it not being a default SPI pin */
void S25FL256_Disable()
{
    //set CS pin to high
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
}

/* READ WRITE TEST 
*writes 1 block en reads it
* returns 1 if equal (success), 0 if not equal (fail)
*/
uint8_t S25FL256_TestRW()
{
    //test flash RW
    #define SIZE 256
    #define BLOCK_ID 0

    uint8_t block1 = 1;
    uint8_t block2 = 1;
    uint8_t block3 = 1;
    

    static uint8_t tx[SIZE];
    static uint8_t rx[SIZE];

    //fill tx array
    for (int n = 0; n < SIZE; n++)
    {
        tx[n] = n;
    }

    S25FL256_open(BLOCK_ID);

    S25FL256_write((uint8_t *)tx, SIZE);

    while (S25FL256_isWriteInProgress())
    {
        HAL_Delay(SPI_WAIT);
    }

    S25FL256_read((uint8_t *)rx, SIZE);
    
    for(int i = 0; i < SIZE; i++)
    {
        if(tx[i] != rx[i])
        {
            block1 =0;
        }
    }

    for (int n = 0; n < SIZE; n++)
    {
        tx[n] = 255-n;
    }
    S25FL256_open(BLOCK_ID+1);

    S25FL256_write((uint8_t *)tx, SIZE);

    while (S25FL256_isWriteInProgress())
    {
        HAL_Delay(SPI_WAIT);
    }

    S25FL256_read((uint8_t *)rx, SIZE);
    
    for(int i = 0; i < SIZE; i++)
    {
        if(tx[i] != rx[i])
        {
            block2 = 0;
        }
    }
    for (int n = 0; n < SIZE; n++)
    {
        tx[n] = n/2;
    }
    S25FL256_open(BLOCK_ID+2);

    S25FL256_write((uint8_t *)tx, SIZE);

    while (S25FL256_isWriteInProgress())
    {
        HAL_Delay(SPI_WAIT);
    }

    S25FL256_read((uint8_t *)rx, SIZE);
    
    for(int i = 0; i < SIZE; i++)
    {
        if(tx[i] != rx[i])
        {
            block3 = 0;
        }
    }
    if(block1 && block2 && block3)
        return 1;
    else
        return 0;
}