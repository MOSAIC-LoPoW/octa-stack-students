#include "platform.h"
#include "NORDIC9160.h"

char imsi[16];
struct OCTA_header NB_IoTHeader;
UART_HandleTypeDef *uartHandle;
struct OCTA_GPIO *nb_iot_reset_pin_nordic;
bool mpUART = false;
bool nordic_hw_initialized, nordic_initialized = false;
bool ublox_chip = false;
char m_server_ip[16];
uint32_t m_server_port;
uint8_t m_socket;
uint64_t m_imsi;

uint8_t rxLastValue;
char rxBuffer[NORDIC9160_MAX_RX_BUFFER_SIZE];
uint16_t rxBufferLen;
uint8_t appRxBuffer[NORDIC9160_MAX_RX_BUFFER_SIZE];
uint16_t appRxBufferLen;
const char *R_server_ip;
const uint32_t R_server_port;
void (*applicationCallback)(uint8_t *buffer, uint16_t len) = 0;

bool NORDIC9160_Initialize(void) {

	if(nordic_hw_initialized) {
      return false;
    }

    printINF("***Initializing NB-IoT driver***\r\n");

	//check if platform is octa stm with onboad nrf9160 chip
    #ifdef platform_octa_stm
		printINF("Platform octa-stm, using NB-IoT UART\r\n");
		NBIOT_UART_Init(NORDIC9160_BAUDRATE);
		uartHandle = &NBIOT_UART;
		nb_iot_reset_pin_nordic = &NBIOT_Reset;
		mpUART = true;
		UART_NBIOT_SetRxCallback(&NORDIC9160_rxCallback);
    //else check connector
    #else
		#ifndef NB_IOT_CONNECTOR
			printERR("No NB_IOT_CONNECTOR provided in Makefile\r\n");
			return false;
		#else
			NB_IoTHeader = platform_getHeader((uint8_t)NB_IOT_CONNECTOR);
			if(!NB_IoTHeader.active)
			{
				printERR("Invalid NB_IOT_CONNECTOR provided in Makefile\r\n");
				return 0;  
			}
			else
			{
				printINF("NB-IoT on P%d, initializing UART\r\n", (uint8_t)NB_IOT_CONNECTOR);    
				// Initialize UART peripheral with driver baudrate
				platform_initialize_UART(NB_IoTHeader, NORDIC9160_BAUDRATE);

				uartHandle = NB_IoTHeader.uartHandle;
				nb_iot_reset_pin_nordic = NB_IoTHeader.DIO2;
				mpUART = true;    
				UART_SetShieldCallback(&NORDIC9160_rxCallback, (uint8_t)NB_IOT_CONNECTOR);
			}
		#endif
	#endif

	NORDIC9160_toggleResetPin();

	nordic_hw_initialized = true;
	return nordic_hw_initialized;
}

void NORDIC9160_toggleResetPin(void)
{
	HAL_GPIO_WritePin(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(nb_iot_reset_pin_nordic->PORT, nb_iot_reset_pin_nordic->PIN, GPIO_PIN_SET);
	nordic_initialized = false;
	HAL_Delay(1000);
}

void NORDIC9160_set_server_parameters(const char* server_ip, const uint32_t server_port)
{
	m_server_port = server_port;
	sprintf(m_server_ip, server_ip);
}

bool NORDIC9160_init_module(const char* server_ip, const uint32_t server_port) 
{
	char ip[16];
	uint8_t socket;
	IWDG_feed(NULL);

    m_server_port = server_port;
	sprintf(m_server_ip, server_ip);

	printDBG("Initializing NORDIC-9160 NB-IoT module...\r\n");

	IWDG_feed(NULL);

	if (nordic_initialized) {
		printERR("NORDIC-9160 NB-IoT module is already initialized!\r\n");
		return false;
	}

	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_set_band()) {
		printERR("Cannot set band in the module! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}  
	
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if(!NORDIC9160_turn_on()) 
	{
		printERR("Cannot turn on the module! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_set_pdp_context()) {
		printERR("Cannot set PDP context in the module! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_select_plmn()) {
		printERR("Cannot select PLMN in the module! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_wait_for_registering()) {
		printERR("Cannot register the module to the network! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_get_imsi(imsi)) {
		printERR("Cannot get module IMSI! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!NORDIC9160_UDP_connect()) {
		printERR("Cannot connect as UDP client! Initializing NORDIC9160 NB-IoT module failed!\r\n");
		return false;
	}

	printINF("Module %s is successfully registered to %s network.\r\n", imsi, NB_IOT_APN);
	
	printDBG("NORDIC9160 NB-IoT module init OK\r\n\r\n");

	m_socket = socket;
	nordic_initialized = true;
	return true;
}

 bool NORDIC9160_turn_on(void) {
	if (mpUART) {
		const char* cmd = "AT+CFUN=1\r\n";
		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
			for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "OK") != NULL){
					    printINF("[NORDIC9160::turn_on] OK\r\n");
						return true;
					}
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::turn_on] Error while turning on the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::turn_on] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_set_pdp_context(void) //The +CGDCONT command defines Packet Data Protocol (PDP) Context.
{
	if (mpUART) {
		char cmd[60];
		sprintf(cmd, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", NB_IOT_APN);
		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
			for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "OK") != NULL)
					{
						printINF("[NORDIC9160::set_pdp_context] OK\r\n");
						return true;
					}
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::set_pdp_context] Error while setting PDP context in the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::set_pdp_context] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_set_band(void)  // The command issues a valid response only when the modem is activated.
{
	if (mpUART) {
		char cmd[50];
		sprintf(cmd, NB_IOT_BAND);
		
		char rxData[16];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "OK") != NULL)
					{
						printINF("[NORDIC9160::set_band] OK\r\n");
						return true;
					}	
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::set_band] Error while setting the band in the module!\r\n");
						printDBG("RX: %s\r\n",rxData);
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::set_band] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_select_plmn(void) {
	if (mpUART) {
		char cmd[60]; 
		sprintf(cmd, "AT+COPS=1,2,\"%s\"\r\n", NB_IOT_PLMN);
		char rxData[16];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 16, false, 3000)) {
					if (strstr(rxData, "OK") != NULL)
					{
						printINF("[NORDIC9160::select_plmn] OK\r\n");
						return true;
					}
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::select_plmn] Error while selecting PLMN in the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::select_plmn] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_get_imsi(char* imsi) {
	if (!imsi) {
		printERR("[NORDIC9160::get_imsi] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+CIMI\r\n"; 
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 32, false, 500)) {
					if (strstr(rxData, "OK") != NULL) {
						uint8_t index = 0;

						for (uint8_t indexRef = 0; indexRef < strlen(rxData);
								indexRef++) {
							if ((rxData[indexRef] <= '9')
									&& (rxData[indexRef] >= '0'))
								imsi[index++] = rxData[indexRef];
						}
						imsi[index] = '\0';
						imsi_number.u64 = NORDIC9160_convert_imsi_char_to_u64(imsi);
						return true;
					} else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::get_imsi] Error while getting module IMSI!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::get_imsi] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_UDP_disconnect(void)  // The command issues a valid response only when the modem is activated.
{
	if (mpUART) {
		char cmd[50];
		sprintf(cmd, "AT#XUDPCLI=0,\"%s\",%d\r\n", m_server_ip, m_server_port);
		
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 32, false, 1000)) {
					if (strstr(rxData, "OK") != NULL)
					{
						printINF("[NORDIC9160::udp_disconnect] OK\r\n");
						return true;
					}	
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::udp_disconnect] Error while disconnecting to UDP server!\r\n");
						printDBG("RX: %s\r\n",rxData);
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::udp_disconnect] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_UDP_connect(void)  // The command issues a valid response only when the modem is activated.
{
	if (mpUART) {
		char cmd[50];
		sprintf(cmd, "AT#XUDPCLI=1,\"%s\",%d\r\n", m_server_ip, m_server_port);
		
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 32, false, 1000)) {
					if (strstr(rxData, "OK") != NULL)
					{
						printINF("[NORDIC9160::udp_connect] OK\r\n");
						return true;
					}	
					else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::udp_connect] Error while connecting to UDP server!\r\n");
						printDBG("RX: %s\r\n",rxData);
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::udp_connect] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_wait_for_registering(void) 
{
	for (uint8_t cpt = 0; cpt < NORDIC9160_WAIT_FOR_REGISTERING_ATTEMPTS; cpt++) 
	{
        WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
		uint8_t status = NORDIC9160_get_registration_status();

		if (status == REGISTERED)
			return true;
		else if ((status == NOT_REGISTERED) || (status == NB_IOT_ERROR))
			return false;
		if (cpt >= (NORDIC9160_WAIT_FOR_REGISTERING_ATTEMPTS - 1))
			return false;
		HAL_Delay(1000);
	}
	return false;
}

uint8_t NORDIC9160_get_registration_status(void) {
	if (mpUART) {
		const char* cmd = "AT+CEREG?\r\n";
		char rxData[32];
		char rxData1[32];
		uint8_t status = NB_IOT_ERROR;

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
                WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (NORDIC9160_read_line(rxData, 32, false, 1000)) 
				{
					if (strstr(rxData, "+CEREG") != NULL) 
					{

						for (uint8_t index = 0; index < strlen(rxData);index++) {

							if ((rxData[index] <= '9') && (rxData[index] >= '0')) 
							{
								if (status == NB_IOT_ERROR) 
								{
									status = rxData[index];
								} else {
									status = rxData[index];
									break;
								}
							}
						}
						switch (status) {
						case '0':
							status = NOT_REGISTERED;
                            printDBG("NOT REGISTERED \r\n");
							break;
						case '1':
							status = REGISTERED;
                            printDBG("REGISTERED \r\n");
                            break;
						case '2':
							status = REGISTERING;
                            printDBG("REGISTERING \r\n");
							break;
						default:
							status = NB_IOT_ERROR;
                            printDBG("NB IoT REG ERROR \r\n");
							break;
						}
						return status;
					} else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::get_registration_status] Error while getting module registration status!\r\n");
						return status;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("NORDIC9160::get_registration_status] Error: Timeout!\r\n");
					return status;
				}
			}
		}
	}
	return NB_IOT_ERROR;
}

bool NORDIC9160_get_cell_id(char* cell) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!cell) {
		printERR("[NORDIC9160::get_cell_id] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+NUESTATS\r\n";
		char rxData[200];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 200, false, 1000)) {
					if (strstr(rxData, "##OK##") != NULL) {
						char *cell_pos = strstr(rxData, "Cell ID")
								+ strlen("Cell ID") + 2;
						uint8_t cell_len = (uint8_t)(
								strstr(rxData, "##\"ECL") - cell_pos);
						uint8_t index;
						for (index = 0; index < cell_len; index++) {
							cell[index] = cell_pos[index];
						}
						cell[index] = '\0';
						printDBG("cell_id %s \r\n", cell);
						return true;
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printERR("[NORDIC9160::get_cell_id] Error while getting module cell ID!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::get_cell_id] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_get_nuestats(char* cell, char* txpower) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!cell && !txpower) {
		printERR("[NORDIC9160::get_nuestats] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+NUESTATS\r\n";
		char rxData[200];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 200, false, 1000)) {
					if (strstr(rxData, "OK") != NULL) {
						if (ublox_chip) {
							//CELL ID
							char *cell_pos = strstr(rxData, "Cell ID")
									+ strlen("Cell ID") + 2;
							uint8_t cell_len = (uint8_t)(
									strstr(rxData, "##\"ECL") - cell_pos);
							uint8_t index;
							for (index = 0; index < cell_len; index++) {
								cell[index] = cell_pos[index];
							}
							cell[index] = '\0';
							printDBG("cell_id %s \r\n", cell);

							//TX POWER
							char *tx_pos = strstr(rxData, "TX power")
									+ strlen("TX power") + 2;
							uint8_t tx_len = (uint8_t)(
									strstr(rxData, "##\"TX time") - tx_pos);
							for (index = 0; index < tx_len; index++) {
								txpower[index] = tx_pos[index];
							}
							txpower[index] = '\0';
							printDBG("txpower %s \r\n", txpower);
							return true;
						}
						else{
							//CELL ID
							char *cell_pos = strstr(rxData, "Cell ID")
									+ strlen("Cell ID") + 1;
							uint8_t cell_len = (uint8_t)(
									strstr(rxData, "##DL MCS") - cell_pos);
							uint8_t index;
							for (index = 0; index < cell_len; index++) {
								cell[index] = cell_pos[index];
							}
							cell[index] = '\0';
							printDBG("cell_id %s \r\n", cell);

							//TX POWER
							char *tx_pos = strstr(rxData, "TX power")
									+ strlen("TX power") + 1;
							uint8_t tx_len = (uint8_t)(
									strstr(rxData, "##TX time") - tx_pos);
							for (index = 0; index < tx_len; index++) {
								txpower[index] = tx_pos[index];
							}
							txpower[index] = '\0';
							printDBG("txpower %s \r\n", txpower);
							return true;
						}
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printERR("[NORDIC9160::get_nuestats] Error while getting module cell ID!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::get_nuestats] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_get_RSSI(char* RSSI) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!RSSI) {
		printERR("[NORDIC9160::get_RSSI] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+CSQ\r\n";
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < NORDIC9160_READ_UART_ATTEMPTS; cpt++) {
				if (NORDIC9160_read_line(rxData, 32, false, 500)) {
					if (strstr(rxData, "OK") != NULL) {
						if(ublox_chip){
							//RSSI
							char *RSSI_pos = strstr(rxData, "+CSQ:")
									+ strlen("+CSQ:") + 1;

							uint8_t RSSI_len = (uint8_t)(
									strstr(rxData, ",") - RSSI_pos);

							uint8_t index;
							//len of RSSI is always 2 chars
							for (index = 0; index < RSSI_len; index++) {
								RSSI[index] = RSSI_pos[index];
							}
							RSSI[index] = '\0';
							printDBG("rssi %s \r\n", RSSI);

							return true;
						}
						else{
							//RSSI
							char *RSSI_pos = strstr(rxData, "+CSQ:")
									+ strlen("+CSQ:");

							uint8_t index;
							//len of RSSI is always 2 chars
							for (index = 0; index < 2; index++) {
								RSSI[index] = RSSI_pos[index];
							}
							RSSI[index] = '\0';
							printDBG("rssi %s \r\n", RSSI);

							return true;

						}
					} else if (strstr(rxData, "ERROR") != NULL) {
						printERR("[NORDIC9160::get_RSSI] Error while getting module RSSI!\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (NORDIC9160_READ_UART_ATTEMPTS - 1)) {
					printERR("[NORDIC9160::get_RSSI] Error: Timeout!\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool NORDIC9160_send(uint8_t* msg, uint16_t length) {
	return NORDIC9160_send_to_server(msg, length, m_server_ip, m_server_port);
}

bool NORDIC9160_send_to_server(uint8_t *msg, uint16_t length, const char *server_ip, const uint32_t server_port) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	R_server_ip = server_ip;

	char rxData[20];
	
	if (mpUART) {
		if (!msg) {
			printERR("[NORDIC9160::send] Error: Invalid input pointer!\r\n");
			return false;
		}

		if (!nordic_initialized) {
			printERR("[NORDIC9160::send] Error: module not initialized!\r\n");
			return false;
		}
		
		char *cmd = (char *) malloc(20 + length*2);

		sprintf(cmd, "AT#XUDPSEND=0,\"");
		for (uint16_t cpt = 0; cpt < length; cpt++) {
			sprintf(cmd, "%s%02x", cmd, msg[cpt]);
		}
		sprintf(cmd, "%s\"\r\n", cmd);

		bool result = (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, (uint16_t)strlen(cmd), 200)==HAL_OK);
		NORDIC9160_read_line(rxData, 18, false, 200);
		if(strstr(rxData, "OK") && strstr(rxData, "#XUDPSEND: "))
		{
			printINF("NB-IoT message sent with payload size %d to %s:%d\r\n", length, server_ip, server_port);
			return true;
		}
		else
		{
			printERR("[NORDIC9160::send] Error: Send not OK\r\n");
			return false;
		}
	}
	return false;
}

uint16_t NORDIC9160_read_line(char* ptr, uint16_t len, bool debug, uint32_t delay) {
	size_t index = 0;
	uint8_t tmpBuf[len];

	if (!ptr) {
		printERR("[NORDIC9160::read_line] Error: Invalid input pointer!\r\n");
		return 0;
	}
	memset(tmpBuf, 0, len);
	memset(ptr, 0, len);

	if (!mpUART) {
		printERR("[NORDIC9160::read_line] Error: Invalid UART object!\r\n");
		return 0;
	}
	
	//Disable RX interrupt before blocking read
	HAL_UART_AbortReceive(uartHandle);

	//perform the actual blocking read
	HAL_UART_Receive(uartHandle, (uint8_t *)tmpBuf, len, delay);

	//Re enable the RX interrupt for downlinks
	HAL_UART_Receive_IT(uartHandle, &rxLastValue, 1);
	
	for (uint16_t cpt = 0; cpt < len; cpt++) {
		if ((tmpBuf[cpt] <= 0x7F) && (tmpBuf[cpt] >= 0x20))
			ptr[index++] = tmpBuf[cpt];
		else if ((tmpBuf[cpt] == '\n') || (tmpBuf[cpt] == '\r'))
			ptr[index++] = '#';
	}
	ptr[index] = '\0';

	if (debug) {
		printDBG("read ");
		#if DEBUG
			for (uint16_t cpt = 0; cpt < len; cpt++) {
				printf("%x\t", tmpBuf[cpt]);
			}
		#endif
		printDBG("\nread %s\r\n", ptr);
	}
	return strlen(ptr);
}

void NORDIC9160_Disable_Interrupt()
{
	//Disable RX interrupt before blocking read
	HAL_UART_AbortReceive(uartHandle);
}

uint64_t NORDIC9160_convert_imsi_char_to_u64(const char* text)
{
	uint64_t number=0;

    for(; *text; text++)
    {
        char digit=*text-'0';           
        number=(number*10)+digit;
    }

    return number;
}

void parseHexString(char in[], char out[], int length) 
{
	int outindex = 0;
	char hexValue[2];

	for (int i = 0; i < length / 2; i++) 
	{
		strncpy(hexValue, in + (i * 2), 2);
		int converted = strtol(hexValue, NULL, 16);
		out[outindex] = converted;
		outindex++;
	}
}

void NORDIC9160_setApplicationRxCallback(void (*callback)(uint8_t* buffer, uint16_t len)) {
	if (mpUART && nordic_initialized) 
	{
		applicationCallback = callback;
		memset(rxBuffer, 0, NORDIC9160_MAX_RX_BUFFER_SIZE);
		rxBufferLen = 0;
		memset(appRxBuffer, 0, NORDIC9160_MAX_RX_BUFFER_SIZE);
		appRxBufferLen = 0;
		printDBG("NORDIC9160 NB-IoT RX interrupt set up\r\n");
		HAL_UART_AbortReceive(uartHandle);
		HAL_UART_Receive_IT(uartHandle, &rxLastValue, 1);
	}
}

void NORDIC9160_rxCallback(void) {
	HAL_UART_Receive_IT(uartHandle, &rxLastValue, 1);
	if (((rxLastValue <= 0x7F) && (rxLastValue >= 0x20))|| rxLastValue == '\r') {
		rxBuffer[rxBufferLen++] = rxLastValue;
	}
	if ((rxLastValue == '\n') && rxBufferLen>18)
	{
		rxBuffer[rxBufferLen++] = '\0';
		{
			NORDIC9160_handleRxData();
		}
	}
}

void NORDIC9160_handleRxData(void) {
	char* ptr1 = strstr(rxBuffer, "#XUDPRECV: ");
	if (ptr1 != NULL)
	{
		int len = 0;
		len = atoi(strstr(ptr1 + strlen("#XUDPRECV: "), ",") + 1);
		if (len) 
		{
			uint16_t rx_len =(len <= NORDIC9160_BUFFER_SIZE*2) ? len : NORDIC9160_BUFFER_SIZE*2;
			printDBG("Downlink of %d bytes received\r\n", rx_len/2);
			char* rxbuf_pos = strstr(ptr1 + strlen("#XUDPRECV: "), "\r")+1;
			uint8_t* buffer = malloc(rx_len);
			memset(buffer, 0, rx_len);
			memcpy(buffer, rxbuf_pos, rx_len);
			uint8_t reply[NORDIC9160_BUFFER_SIZE] = {0};
			parseHexString(buffer, reply, rx_len);
			free(buffer);
			if (applicationCallback)
				applicationCallback((uint8_t *)reply, rx_len/2);
			memset(rxBuffer, 0, rxBufferLen);
			rxBufferLen = 0;
			rx_len = 0;
		}
	}
	memset(rxBuffer, 0, rxBufferLen);
	rxBufferLen = 0;
}