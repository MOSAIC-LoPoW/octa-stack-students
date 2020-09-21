#include "stm32l4xx_hal.h"

/* 
NOTES:
    - We can not overwrite flash, i.e. we can only set bits to 0, not one. This is also why an erase sets every bit to one (bytes to 255)
    - The smallest area that can be erased is a 4kB sector
    - To overwrite flash for for example a configuration in flash, we need to reserve an entire 4kB sector. Because we will need to erase it each time before overwriting

MEMORY LAYOUT (octa):
-----------------------------------------------------------------------------------------------------------------------------
index               | purpose                                                                                               |
-----------------------------------------------------------------------------------------------------------------------------
0 - 4095            | configuration sector. Only one page will be used, but we need to reserve an entire sector, see notes  |
-----------------------------------------------------------------------------------------------------------------------------
4096 - 32MB (=256Mb)| can be used for actual data storage, these are 4kB sectors, all consisting of 16 256 byte pages       |
-----------------------------------------------------------------------------------------------------------------------------

MEMORY LAYOUT (octa-stm):
-----------------------------------------------------------------------------------------------------------------------------
index               | purpose                                                                                               |
-----------------------------------------------------------------------------------------------------------------------------
0 - 4095            | configuration sector. Only one page will be used, but we need to reserve an entire sector, see notes  |
-----------------------------------------------------------------------------------------------------------------------------
4096 - 64MB (=512)| can be used for actual data storage, these are 4kB sectors, all consisting of 16 256 byte pages       |
-----------------------------------------------------------------------------------------------------------------------------
*/

#define OCTA_FLASH_PAGE_SIZE    256  /* optimal flash page size*/
#define MIN_FLASH_SIZE			256
#define PAGES_PER_4KBSECTOR     4096/OCTA_FLASH_PAGE_SIZE
#define FIRST_FLASH_ADDRESS     4096
#ifdef platform_octa_stm
    #define LAST_FLASH_ADDRESS      64000000 // == 64 MB (== 512Mb)
#else
    #define LAST_FLASH_ADDRESS      32000000 // == 32 MB (== 256Mb)
#endif
#define MAX_PAGE_NUMBER        ((LAST_FLASH_ADDRESS - FIRST_FLASH_ADDRESS) / OCTA_FLASH_PAGE_SIZE) - 1

#define E_WRITE_IN_PROGRESS     -2
#define E_SPI_ERROR             -3
#define E_INVALID_PAGE_NR       -4
#define E_LAST_PAGE_WRITTEN     -5
#define OCTA_FLASH_OK           0

uint8_t OCTA_FLASH_Initialize(SPI_HandleTypeDef *hspi);
int OCTA_FLASH_open(const uint32_t aPageNr);
int OCTA_FLASH_openConfigSector(void);
int OCTA_FLASH_read(uint8_t *apData, const uint32_t aSize);
int OCTA_FLASH_write(const uint8_t* apData, const uint32_t aSize);
uint8_t OCTA_FLASH_isWriteInProgress(void);
int OCTA_FLASH_eraseFullFlash(void);
int OCTA_FLASH_erase265KBSectorFromPage(const uint32_t aPageNr);
int OCTA_FLASH_erase4KBSectorFromPage(const uint32_t aPageNr);
int OCTA_FLASH_eraseConfigSector(void);
int OCTA_FLASH_flush(void);
int OCTA_FLASH_sendCommand(const uint8_t command);
int OCTA_FLASH_send4BCommand(const uint8_t command, const uint32_t address);
int OCTA_FLASH_readReg(const uint8_t aReg, uint8_t *aData);
uint8_t OCTA_FLASH_TestRW(void);
void OCTA_FLASH_Enable(void);
void OCTA_FLASH_Disable(void);