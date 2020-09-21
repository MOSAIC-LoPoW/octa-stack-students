#include "modem.h"
#ifdef MURATA_Keys_Exists
    #include "keys.h"
#else
    #include "keys_example.h"
#endif
#include "cmsis_os.h"

#define MURATA_BAUDRATE 115200

void on_modem_command_completed_callback(bool with_error, uint8_t tag_id);
void on_modem_return_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t *output_buffer);
void on_modem_write_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t *output_buffer);
void on_modem_interface_status_callback(alp_itf_id_t interface_type, uint8_t* data);
void on_modem_reboot_callback(void);
void modem_wakeup(void);
void modem_release(void);
void Murata_toggleResetPin(void);
uint8_t Murata_Initialize(uint64_t octa_UID);
uint8_t Murata_SetProcessingThread(osThreadId aThreadId);
uint8_t Murata_LoRaWAN_Join(void);
uint8_t Murata_LoRaWAN_Send(uint8_t *buffer, uint8_t length);
uint8_t Murata_Dash7_Send(uint8_t *buffer, uint8_t length);
void Murata_rxCallback(void);
uint8_t Murata_process_fifo(void);

struct OCTA_GPIO *murata_wakeup_pin;
struct OCTA_GPIO *murata_reset_pin;
osThreadId threadToNotify;