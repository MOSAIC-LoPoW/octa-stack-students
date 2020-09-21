#include "octa-flash.h"
#include "platform.h"

#define SPI_WRDI_CMD 0x04 // Write disable
#define SPI_RDSR_CMD 0x05 // Read Status Register
#define SPI_RDCR_CMD 0x35 // Read Status Register
#define SPI_WREN_CMD 0x06 // Write enable

#define SPI_PP_4B_CMD 0x12   //Page Program (4-byte address). Enables writing
#define SPI_READ_4B_CMD 0x13 //Read (4-byte Address)

#define SPI_BE_CMD 0x60 //Bulk erase
#define SPI_SE256KB_CMD 0xDC //Sector erase
#define SPI_SE4KB_CMD 0x20

#define WRITE_IN_PROGRESS_MASK 0x01 //Bit 0 of RDSR is "Write in Progress"

#define SPI_WAIT 10

uint32_t mCurrentReadAddress;
uint32_t mCurrentWriteAddress;
uint8_t  mMemPage[OCTA_FLASH_PAGE_SIZE];
uint16_t mMemPageIndex;
uint8_t _initialised;

//One because it's only used to read a register
uint8_t rxData[1];
//Five, because the max amount of bytes is 5, in send4BCommand
uint8_t txData[5];

SPI_HandleTypeDef *flashSPI;

/**
    * \brief   Initializes the OCTA_FLASH driver.
    * \returns 0 if init is successful
    * \returns E_INIT_FAILED if SPI could not be configured.
    */
uint8_t OCTA_FLASH_Initialize(SPI_HandleTypeDef *hspi)
{
    mMemPageIndex = 0;
    memset(mMemPage, 0, OCTA_FLASH_PAGE_SIZE);
    mCurrentReadAddress = FIRST_FLASH_ADDRESS;
    mCurrentWriteAddress = FIRST_FLASH_ADDRESS;
    _initialised = 0;

    flashSPI = hspi;

    //set CS pin to High (== disable flash, will be enabled when needed)
    OCTA_FLASH_Disable();
    
    //disable WP and HOLD
    HAL_GPIO_WritePin(OCTA_FLASH_WP_Port, OCTA_FLASH_WP_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OCTA_FLASH_HOLD_Port, OCTA_FLASH_HOLD_Pin, GPIO_PIN_SET);

    #ifdef platform_octa_stm
        printINF("MX25L512: 512Mb (64MB) Flash, first 4kB sector reserved for configuration\r\n");
    #else
        printINF("S25FL256: 256Mb (32MB) Flash, first 4kB sector reserved for configuration\r\n");
    #endif
    printDBG("Max page number: %d, page size: %d, start adr: %d, last adr: %d, pages/4kbsector: %d\r\n", MAX_PAGE_NUMBER, OCTA_FLASH_PAGE_SIZE, FIRST_FLASH_ADDRESS, LAST_FLASH_ADDRESS, PAGES_PER_4KBSECTOR);

    _initialised = 1;

    return _initialised;
}

/**
    * \brief    Sets the read and write address to the start of this page. The max value is defined in MAX_PAGE_NUMBER.
    * \param    aPageNr      The 256 byte page you want to start reading or writing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_PAGE_NR if aPageNr is invalid.
    */
int OCTA_FLASH_open(const uint32_t aPageNr)
{
    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (aPageNr > MAX_PAGE_NUMBER)
    {
        return E_INVALID_PAGE_NR;
    }

    uint32_t newAddress = FIRST_FLASH_ADDRESS + aPageNr * (OCTA_FLASH_PAGE_SIZE);
    mCurrentReadAddress = newAddress;
    mCurrentWriteAddress = newAddress;

    return 0;
}

/**
    * \brief    Sets the read and write address to the start of this page. The max value is defined in MAX_PAGE_NUMBER.
    * \param    aPageNr      The 256 byte page you want to start reading or writing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_PAGE_NR if aPageNr is invalid.
    */
int OCTA_FLASH_openConfigSector(void)
{
    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    uint32_t newAddress = 0;
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
int OCTA_FLASH_read(uint8_t *apData, const uint32_t aSize)
{

    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    //Starting the read.
    int status = 0;
    status = OCTA_FLASH_send4BCommand(SPI_READ_4B_CMD, mCurrentReadAddress);
    if (status!=0)
    {
        printDBG("\r\n FLASH read error: %d\r\n", status);
        return status;
    }

    HAL_SPI_Receive(flashSPI, apData, (uint16_t)aSize, HAL_MAX_DELAY);

    OCTA_FLASH_Disable();
    __HAL_SPI_DISABLE(flashSPI);

    return aSize;
}

/**
    * \brief    Buffers the given array. Once 256 bytes are cached, the data is flushed to flash.
    * \param    apData     Pointer to the uint8_t array with data to write.
    * \param    aSize      The number of bytes to be written.
    * \returns  The number of written bytes.
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    * \returns  E_LAST_PAGE_WRITTEN if the last block was written and no more data can be buffered.
    */
int OCTA_FLASH_write(const uint8_t *apData, const uint32_t aSize)
{
    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (mCurrentWriteAddress > LAST_FLASH_ADDRESS)
    {
        return E_LAST_PAGE_WRITTEN;
    }

    uint32_t bytesLeftToWrite = aSize;

    while (mMemPageIndex + bytesLeftToWrite >= OCTA_FLASH_PAGE_SIZE)
    {

        //The number of bytes we will add to mMemBlock in this loop
        uint32_t bytesToWrite = OCTA_FLASH_PAGE_SIZE - mMemPageIndex;

        //Filling the remainder of the buffer
        memcpy(&mMemPage[mMemPageIndex], apData, OCTA_FLASH_PAGE_SIZE);

        int flushedBytes = OCTA_FLASH_flush();

        //And writing the block of FLASH_BLOCK_SIZE bytes. This also clears mMemBlock and mMemBlockIndex.
        if (flushedBytes < 0)
        {
            //Flush failed
            printDBG("Flush failed\r\n");
            return flushedBytes;
        }

        //Moving the pointer
        apData += bytesToWrite;
        bytesLeftToWrite -= bytesToWrite;
    }

    if (mCurrentWriteAddress > LAST_FLASH_ADDRESS)
    {
        //Flash is full, no more buffering.
        return E_LAST_PAGE_WRITTEN;
    }
    else if (bytesLeftToWrite > 0)
    {
        //We still have some bytes left to cache.
        memcpy(&mMemPage[mMemPageIndex], apData, bytesLeftToWrite);
        bytesLeftToWrite = 0;
    }

    return (aSize-bytesLeftToWrite);
}

/**
    * \brief    Returns the state of the write in process status bit.
    * \returns  true is the flash is performing a write action.
    */
uint8_t OCTA_FLASH_isWriteInProgress(void)
{
    HAL_Delay(SPI_WAIT);
    uint8_t status = 0;
    if (OCTA_FLASH_readReg(SPI_RDSR_CMD, &status) == 0)
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
int OCTA_FLASH_eraseFullFlash(void)
{

    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;        
    }

    //Setting write enable
    if (OCTA_FLASH_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual bulk erase
    if (OCTA_FLASH_sendCommand(SPI_BE_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //And disabling write again.
    if (OCTA_FLASH_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/**
    * \brief    Erases 256kb of data, starting from the given block.
                This method is a write action. Poll with isWriteInProgress()
    * \param    aPageNr   The block you want to start erasing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_PAGE_NR if aPageNr is invalid.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_erase265KBSectorFromPage(const uint32_t aPageNr)
{

    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (aPageNr > MAX_PAGE_NUMBER)
    {
        return E_INVALID_PAGE_NR;
    }

    //Setting write enable
    if (OCTA_FLASH_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual sector erase
    if (OCTA_FLASH_send4BCommand(SPI_SE256KB_CMD, (FIRST_FLASH_ADDRESS + aPageNr * (OCTA_FLASH_PAGE_SIZE))) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    //Disabling write.
    if (OCTA_FLASH_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/**
    * \brief    Erases 4kb of data, starting from the given block.
                This method is a write action. Poll with isWriteInProgress()
    * \param    aPageNr   The block you want to start erasing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_PAGE_NR if aPageNr is invalid.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_erase4KBSectorFromPage(const uint32_t aPageNr)
{

    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    if (aPageNr > MAX_PAGE_NUMBER)
    {
        return E_INVALID_PAGE_NR;
    }

    //Setting write enable
    if (OCTA_FLASH_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual sector erase
    if (OCTA_FLASH_send4BCommand(SPI_SE4KB_CMD, (FIRST_FLASH_ADDRESS + aPageNr * (OCTA_FLASH_PAGE_SIZE))) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    //Disabling write.
    if (OCTA_FLASH_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/**
    * \brief    Erases 4kb of data, starting from the given block.
                This method is a write action. Poll with isWriteInProgress()
    * \param    aPageNr   The block you want to start erasing from.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_INVALID_PAGE_NR if aPageNr is invalid.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_eraseConfigSector()
{

    HAL_Delay(SPI_WAIT);

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    //Setting write enable
    if (OCTA_FLASH_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //The actual sector erase
    if (OCTA_FLASH_send4BCommand(SPI_SE4KB_CMD, 0) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    //Disabling write.
    if (OCTA_FLASH_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/**
    * \brief    Reads the value of a register.
    * \param    aReg      The register ytou want to read
    * \param    aData     Pointer to the data location.
    * \returns  0 if successful
    * \returns E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_readReg(uint8_t aReg, uint8_t *aData)
{
    txData[0] = aReg;
    *aData = 0x00;

    OCTA_FLASH_Enable();
    __HAL_SPI_ENABLE(flashSPI);

    HAL_SPI_Transmit(flashSPI, (uint8_t *)txData, 1, HAL_MAX_DELAY);

    HAL_SPI_Receive(flashSPI, (uint8_t *)rxData, 1, HAL_MAX_DELAY);

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    *aData = rxData[0];

    return 0;
}

/**
    * \brief   Flushes the internal buffer and writes it to flash.
       This method should only be called when mMemBlock is full.
    * \returns 0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_flush(void)
{
    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    //Setting write enable
    if (OCTA_FLASH_sendCommand(SPI_WREN_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    // write command and address adress
    if (OCTA_FLASH_send4BCommand(SPI_PP_4B_CMD, mCurrentWriteAddress) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    if(HAL_SPI_Transmit(flashSPI, (uint8_t *)mMemPage, OCTA_FLASH_PAGE_SIZE, HAL_MAX_DELAY) != HAL_OK)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    //Setting write disable
    if (OCTA_FLASH_sendCommand(SPI_WRDI_CMD) == E_SPI_ERROR)
    {
        return E_SPI_ERROR;
    }

    //It's possible that we pass LAST_FLASH_ADDRESS here, but the write method checks for that.
    mCurrentWriteAddress += OCTA_FLASH_PAGE_SIZE;

    // Clearing mMemBlock and mMemBlockIndex
    memset(mMemPage, 0, OCTA_FLASH_PAGE_SIZE);
    mMemPageIndex = 0;

    return 0;
}

/**
    * \brief    Sends a single command to the flash
    * \param    command The command to be sent.
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_sendCommand(const uint8_t command)
{

    if (OCTA_FLASH_isWriteInProgress())
    {
        return E_WRITE_IN_PROGRESS;
    }

    txData[0] = command;

    OCTA_FLASH_Enable();
    __HAL_SPI_ENABLE(flashSPI);

    if(HAL_SPI_Transmit(flashSPI, (uint8_t *)txData, 1, HAL_MAX_DELAY)!= HAL_OK)
    {
        return E_SPI_ERROR;
    }

    __HAL_SPI_DISABLE(flashSPI);
    OCTA_FLASH_Disable();

    return 0;
}

/**
    * \brief    Sends a 4 byte address command
    * \param    command   The command to be sent.
    * \param    address  The 4 byte address
    * \returns  0 if successful
    * \returns  E_WRITE_IN_PROGRESS if the flash is performing a write action.
    * \returns  E_SPI_ERROR if something went wrong during SPI communication.
    */
int OCTA_FLASH_send4BCommand(const uint8_t command, const uint32_t address)
{

    if (OCTA_FLASH_isWriteInProgress())
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
    OCTA_FLASH_Enable();
    __HAL_SPI_ENABLE(flashSPI);

    //send the command over SPI
    if(HAL_SPI_Transmit(flashSPI, (uint8_t *)txData, index, HAL_MAX_DELAY)!=HAL_OK)
    {
        return E_SPI_ERROR;
    }

    return 0;
}

/* Manually toggle enable pin, due to it not being a default SPI pin */
void OCTA_FLASH_Enable(void)
{
    //set CS pin to low
   HAL_GPIO_WritePin(OCTA_FLASH_CS_Port, OCTA_FLASH_CS_Pin, GPIO_PIN_RESET);
}

/* Manually toggle enable pin, due to it not being a default SPI pin */
void OCTA_FLASH_Disable(void)
{
    //set CS pin to high
    HAL_GPIO_WritePin(OCTA_FLASH_CS_Port, OCTA_FLASH_CS_Pin, GPIO_PIN_SET);
}

/*  READ WRITE TEST 
*   writes 1 block en reads it
*   returns 1 if equal (success), 0 if not equal (fail)
*/
uint8_t OCTA_FLASH_TestRW(void)
{
    //test flash RW
    #define SIZE 256
    
    static uint8_t tx[SIZE] = {0};
    static uint8_t rx[SIZE] = {0};
    uint16_t i = 0;

    uint8_t page0 = 1;
    uint8_t page1 = 1;
    uint8_t page2 = 1;

    OCTA_FLASH_open(0);
    #if DEBUG
        OCTA_FLASH_read(rx,SIZE);
        printDBG("Page0 before: \r\n");
        for(i = 0; i<256; i++)
        {   
            printf("%d ", rx[i]);
        }        
        printf("\r\n"); 
    #endif
    
    memset(tx, 0, SIZE);
    #if DEBUG
        printDBG("tx: \r\n");
        for(i = 0; i<SIZE; i++)
            printf("%d ", tx[i]);
        printf("\r\n");
    #endif

    OCTA_FLASH_write(tx, SIZE);
    while(OCTA_FLASH_isWriteInProgress())
        HAL_Delay(SPI_WAIT);

    OCTA_FLASH_read(rx,SIZE);
    printDBG("Page0 after: \r\n");
    for(i = 0; i<SIZE; i++)
    {
        #if DEBUG
            printf("%d ", rx[i]); 
        #endif
        if(rx[i] != tx[i])
        {
            page0 = 0;
            printERR("Page0 NO MATCH\r\n");
        }
            
    }
    #if DEBUG
        printf("\r\n"); 
    #endif
    
    OCTA_FLASH_open(1);
    #if DEBUG
        OCTA_FLASH_read(rx,SIZE);
        printDBG("Page1 before: \r\n");
        for(i = 0; i<256; i++)
            printf("%d ", rx[i]); 
        printf("\r\n");
    #endif 

    memset(tx, 1, SIZE);
    #if DEBUG
        printDBG("tx: \r\n");
        for(i = 0; i<SIZE; i++)
            printf("%d ", tx[i]);
        printf("\r\n");
    #endif

    OCTA_FLASH_write(tx, SIZE);
    while(OCTA_FLASH_isWriteInProgress())
        HAL_Delay(SPI_WAIT);

    OCTA_FLASH_read(rx,SIZE);
    printDBG("Page1 after: \r\n");
    for(i = 0; i<256; i++)
    {
        #if DEBUG
            printf("%d ", rx[i]);
        #endif
        if(rx[i] != tx[i])
        {
            page1 = 0;
            printERR("Page1 NO MATCH\r\n");
        }
    }
    #if DEBUG
        printf("\r\n"); 
    #endif

    OCTA_FLASH_open(2);
    #if DEBUG
        OCTA_FLASH_read(rx,SIZE);
        printDBG("Page1 before: \r\n");
        for(i = 0; i<256; i++)
            printf("%d ", rx[i]); 
        printf("\r\n");
    #endif 

    memset(tx, 2, SIZE);
    #if DEBUG
        printDBG("tx: \r\n");
        for(i = 0; i<SIZE; i++)
            printf("%d ", tx[i]);
        printf("\r\n");
    #endif

    OCTA_FLASH_write(tx, SIZE);
    while(OCTA_FLASH_isWriteInProgress())
        HAL_Delay(SPI_WAIT);

    OCTA_FLASH_read(rx,SIZE);
    printDBG("Page2 after: \r\n");
    for(i = 0; i<256; i++)
    {
        #if DEBUG
            printf("%d ", rx[i]);
        #endif
        if(rx[i] != tx[i])
        {
            page2 = 0;
            printERR("Page2 NO MATCH\r\n");
        }
    }
    #if DEBUG
        printf("\r\n"); 
    #endif

    return (page0 && page1 && page2);
}