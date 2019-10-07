/**
 * \file        SARAN210.cpp
 * \copyright   Copyright (c) 2018 Imec. All rights reserved.
 *              Redistribution and use in source or binary form,
 *              with or without modification is prohibited.
 *
 * \class       SARAN210
 *
 * \details     Driver for SARAN210 NB-IoT module.
 *
 * \author      Mahfoudhi Farouk <farouk.mahfoudhi@uantwerpen.be>
 * \date        08-2018
 */
#include "platform.h"
#include "SARAN21X.h"


char imsi[16];
struct OCTA_header NB_IoTHeader;
UART_HandleTypeDef *uartHandle;
bool mpUART = false;
bool m_initialized;
bool ublox_chip = false;
char m_server_ip[16];
uint32_t m_server_port;
uint8_t m_socket;
uint64_t m_imsi;
uint8_t SARAN_notification_sent = 0;
void (*applicationCallback)(uint8_t *buffer, uint16_t len);

uint8_t rxLastValue;
char rxBuffer[SARAN210_MAX_RX_BUFFER_SIZE];
uint16_t rxBufferLen;
uint8_t appRxBuffer[SARAN210_MAX_RX_BUFFER_SIZE];
uint16_t appRxBufferLen;

void parseHexString(char in[], char out[], int length) {
	int outindex = 0;
	char hexValue[2];

	for (int i = 0; i < length / 2; i++) {
		strncpy(hexValue, in + (i * 2), 2);
		int converted = strtol(hexValue, NULL, 16);
		out[outindex] = converted;
		outindex++;
	}
}

/************************************************************************/
/* Public Methods                                                       */

/************************************************************************/


bool SARAN21X_Initialize(void) {
    printf("***Initializing NB-IoT driver***\r\n");

	#ifndef NB_IOT_CONNECTOR
        printf("No NB_IOT_CONNECTOR provided in Makefile\r\n");
        return 1;
    #else
        NB_IoTHeader = platform_getHeader((uint8_t)NB_IOT_CONNECTOR);
		if(!NB_IoTHeader.active)
		{
			printf("Invalid NB_IOT_CONNECTOR provided in Makefile\r\n");
            return 0;  
		}
		else
            printf("NB-IoT on P%d, initializing UART\r\n", (uint8_t)NB_IOT_CONNECTOR);        
    #endif

	// Initialize UART peripheral with driver baudrate
    platform_initialize_UART(NB_IoTHeader, SARAN210_BAUDRATE);


	uartHandle = NB_IoTHeader.uartHandle;
	nb_iot_reset_pin = NB_IoTHeader.DIO1;
    mpUART = true;

	return true;
}

void SARAN21X_toggleResetPin(void){
	HAL_GPIO_WritePin(nb_iot_reset_pin->PORT, nb_iot_reset_pin->PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(nb_iot_reset_pin->PORT, nb_iot_reset_pin->PIN, GPIO_PIN_SET);
	m_initialized = false;
}

void SARAN21X_setApplicationRxCallback(
		void (*callback)(uint8_t* buffer, uint16_t len)) {
	if (mpUART && m_initialized) {
		applicationCallback = callback;
		memset(rxBuffer, 0, SARAN210_MAX_RX_BUFFER_SIZE);
		rxBufferLen = 0;
		memset(appRxBuffer, 0, SARAN210_MAX_RX_BUFFER_SIZE);
		appRxBufferLen = 0;
		//mSARAN210 = this;
        //mpUART->read(&rxLastValue, (size_t) 1, SARAN21X_rxCallback);
		HAL_UART_Receive_IT(uartHandle, &rxLastValue, 1);
		printf("NB IoT RX interrupt set up\r\n");\
	}
}

void SARAN21X_rxCallback(void) {
	HAL_UART_Receive_IT(uartHandle, &rxLastValue, 1);
	if ((rxLastValue <= 0x7F) && (rxLastValue >= 0x20)) {
		rxBuffer[rxBufferLen++] = rxLastValue;
	}

	if (((rxLastValue == '\n') || (rxLastValue == '\r')) && rxBufferLen)
	{
		rxBuffer[rxBufferLen++] = '\0';
		{
			SARAN21X_handleRxData();
		}
	}
}

void SARAN21X_handleRxData(void) {
	char* ptr1 = strstr(rxBuffer, "+NSONMI:");

	if (ptr1 != NULL) {
		int len = 0;
		if(ublox_chip)
		{
			len = atoi(strstr(ptr1 + strlen("+NSONMI: "), ",") + 1);
		}
		else
		{
			len = atoi(strstr(ptr1 + strlen("+NSONMI:"), ",") + 1);
		}
		

		if (len) {
			uint16_t rx_len =
					(len <= SARAN210_BUFFER_SIZE) ? len : SARAN210_BUFFER_SIZE;
			printf("Receiving downlink of %d bytes\r\n", rx_len);
			SARAN21X_receive(rx_len);
		}
		memset(rxBuffer, 0, rxBufferLen);
		rxBufferLen = 0;
		return;
	}
	char prefix[32];
	sprintf(prefix, ",%d,", m_server_port);
	ptr1 = strstr(rxBuffer, prefix);

	if (ptr1 != NULL) {
		char* ptr2 = strstr(ptr1 + strlen(prefix), ",");
		if(ublox_chip)
		{
			ptr2++;
		}	

		if (ptr2 != NULL) {
			char lengthStr[10];

			memcpy(lengthStr, ptr1 + strlen(prefix),
					ptr2 - (ptr1 + strlen(prefix)));
			lengthStr[ptr2 - (ptr1 + strlen(prefix))] = '\0';
			int len = atoi(lengthStr);

			if (len) {
				char *ptr3 = strstr(ptr2 + 1 + ublox_chip, ",");

				if (ptr3 != NULL) {
					int rest = atoi(ptr3 + strlen(","));
					char inBuf[2 * SARAN210_BUFFER_SIZE];
					char outBuf[SARAN210_BUFFER_SIZE];

					memcpy(inBuf, ptr2 + strlen(","), 2 * len);
					parseHexString(inBuf, outBuf, 2 * len);
					memcpy(appRxBuffer + appRxBufferLen, outBuf, len);
					appRxBufferLen += len;

					if (rest) {
						SARAN21X_receive(SARAN210_BUFFER_SIZE);
					} else {
						printf("Data received by SARAN210 module.\r\n");
						for (int i = 0; i < appRxBufferLen; ++i) {
							printf("%02x ", appRxBuffer[i]);
						}
						printf("\r\n");
						//applicationCallback((uint8_t *) appRxBuffer, appRxBufferLen);
						memset(rxBuffer, 0, rxBufferLen);
						rxBufferLen = 0;
						memset(appRxBuffer, 0, appRxBufferLen);
						appRxBufferLen = 0;
						return;
					}
				}
			}
		}
	}
	memset(rxBuffer, 0, rxBufferLen);
	rxBufferLen = 0;
}

bool SARAN21X_init_module(const char* server_ip, const uint32_t server_port) {
	char ip[16];
	uint8_t socket;

    m_server_port = server_port;
	sprintf(m_server_ip, server_ip);

	printf("Initializing SARA-N210 NB-IoT module...\r\n");

	if (m_initialized) {
		printf("SARA-N210 NB-IoT module is already initialized!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_reboot()) {
		printf(
				"Cannot reboot the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_turn_on()) {
		printf(
				"Cannot turn on the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_set_pdp_context()) {
		printf(
				"Cannot set PDP context in the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_set_band()) {
		printf(
				"Cannot set band in the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_select_plmn()) {
		printf(
				"Cannot select PLMN in the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_autoconnect()) {
		printf(
				"Cannot autoconnect the module! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_wait_for_registering()) {
		printf(
				"Cannot register the module to the network! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_set_low_power_mode()) {
		printf(
				"Cannot set module low power mode! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_get_imsi(imsi)) {
		printf(
				"Cannot get module IMSI! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_get_ip(ip)) {
		printf(
				"Cannot get module IP address! Initializing SARA-N210 NB-IoT module failed!\r\n");
		return false;
	}
    WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!SARAN21X_open_socket(&socket)) {
		printf(
				"Cannot open socket on port %d! Initializing SARA-N210 NB-IoT module failed!\r\n",
				(int) server_port);
		return false;
	}
	printf(
			"Module %s is successfully registered to %s network (IP=%s)!\r\n",
			imsi, NB_IOT_APN, ip);
	printf("SARA-N210 NB-IoT module init OK\r\n\r\n");

	m_socket = socket;
	m_initialized = true;
	return true;
}

bool SARAN21X_reboot(void) {
	if (mpUART) {
		const char* cmd = "AT+NRB\r\n";
		char rxData[128];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
			HAL_Delay(2000);
        	for (uint8_t cpt = 0; cpt < 8 * SARAN210_READ_UART_ATTEMPTS;
					cpt++) {
                if (SARAN21X_read_line(rxData, 128, false, 1000)) {
					printf("%s\n", rxData);
					if (strstr(rxData, "u-blox") != NULL)
						ublox_chip = true;
					if (ublox_chip && strstr(rxData, "#+UFOTAS") != NULL)
						return true;
					else if (!ublox_chip && strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::reboot] Error while rebooting the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (8 * SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::reboot] Error: Timeout!\r\n");
					return false;
				}
				HAL_Delay(10);
			}
		}
	}
	return false;
}

bool SARAN21X_turn_on(void) {
	if (mpUART) {
		const char* cmd = "AT+CFUN=1\r\n";
		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
			for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::turn_on] Error while turning on the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::turn_on] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_set_pdp_context(void) {
	if (mpUART) {
		char cmd[60];
		sprintf(cmd, "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", NB_IOT_APN);
		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
			for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::set_pdp_context] Error while setting PDP context in the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::set_pdp_context] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_set_band(void) {
	if (mpUART) {
		char cmd[30];
		sprintf(cmd, NB_IOT_BAND);
		char rxData[16];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::set_band] Error while setting the band in the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::set_band] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_select_plmn(void) {
	if (mpUART) {
		char cmd[60]; 
		sprintf(cmd, "AT+COPS=1,2,\"%s\"\r\n", NB_IOT_PLMN);
		char rxData[16];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::select_plmn] Error while selecting PLMN in the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::select_plmn] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_autoconnect(void) {
	if (mpUART) {
		char cmd[60];
		if (ublox_chip) {
			sprintf(cmd, "AT+NCONFIG=\"AUTOCONNECT\",\"%s\"\r\n", NB_IOT_AUTOCONNECT);
			//cmd = "AT+NCONFIG=\"AUTOCONNECT\",\"FALSE\"\r\n";
		} else {
			sprintf(cmd, "AT+NCONFIG=AUTOCONNECT,%s\r\n", NB_IOT_AUTOCONNECT);
			//cmd = "AT+NCONFIG=AUTOCONNECT,FALSE\r\n";
		}

		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
    		for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 550)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::autoconnect] Error while autoconnecting the module!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::autoconnect] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_set_low_power_mode(void) {
	if (mpUART) {
		const char* cmd;
		if (ublox_chip) {
			cmd = "AT+CPSMS=1,,,\"11011111\",\"00000000\"\r\n";
		} else {
			cmd = "AT+CPSMS=1,,,11011111,00000000\r\n";	
		}

		char rxData[16];

        if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
    		for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::low power mode] Error while setting low power mode!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::low power mode] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_get_imsi(char* imsi) {
	if (!imsi) {
		printf("[SARAN210::get_imsi] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+CIMI\r\n";
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 32, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL) {
						uint8_t index = 0;

						for (uint8_t indexRef = 0; indexRef < strlen(rxData);
								indexRef++) {
							if ((rxData[indexRef] <= '9')
									&& (rxData[indexRef] >= '0'))
								imsi[index++] = rxData[indexRef];
						}
						imsi[index] = '\0';
						imsi_number.u64 = SARAN21X_convert_imsi_char_to_u64(imsi);
						return true;
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_imsi] Error while getting module IMSI!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::get_imsi] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_wait_for_registering(void) {
	for (uint8_t cpt = 0; cpt < SARAN210_WAIT_FOR_REGISTERING_ATTEMPTS; cpt++) {
        WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
		uint8_t status = SARAN21X_get_registration_status();

		if (status == REGISTERED)
			return true;
		else if ((status == NOT_REGISTERED) || (status == NB_IOT_ERROR))
			return false;
		if (cpt >= (SARAN210_WAIT_FOR_REGISTERING_ATTEMPTS - 1))
			return false;
		HAL_Delay(1000);
	}
	return false;
}

uint8_t SARAN21X_get_registration_status(void) {
	if (mpUART) {
		const char* cmd = "AT+CEREG?\r\n";
		char rxData[32];
		uint8_t status = NB_IOT_ERROR;

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
                WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (SARAN21X_read_line(rxData, 32, false, 550)) {
					if (strstr(rxData, "##OK##") != NULL) {
						for (uint8_t index = 0; index < strlen(rxData);
								index++) {
							if ((rxData[index] <= '9')
									&& (rxData[index] >= '0')) {
								if (status == NB_IOT_ERROR) {
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
                            printf("NOT REGISTERED \r\n");
							break;
						case '1':
							status = REGISTERED;
                            printf("REGISTERED \r\n");
                            break;
						case '2':
							status = REGISTERING;
                            printf("REGISTERING \r\n");
							break;
						default:
							status = NB_IOT_ERROR;
                            printf("NB IOT ERROR \r\n");
							break;
						}
						return status;
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_registration_status] Error while getting module registration status!\r\n");
						return status;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf(
							"[SARAN210::get_registration_status] Error: Timeout!\r\n");
					return status;
				}
			}
		}
	}
	return NB_IOT_ERROR;
}

bool SARAN21X_get_ip(char* ip) {
	if (!ip) {
		printf("[SARAN210::get_ip] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd0 = "AT+CGPADDR=0\r\n";
		const char* cmd1 = "AT+CGPADDR=1\r\n";
		char rxData[64];
		char tmpBuf[64];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd0, strlen(cmd0), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 64, false, 500)) {
					printf("%s\n", rxData);
					if (strstr(rxData, "##OK##") != NULL) {
						char *got_ip_pos = strstr(rxData, "+CGPADDR:")	+ strlen("+CGPADDR:");

						got_ip_pos++;
						if (ublox_chip)
							got_ip_pos++;

						if (got_ip_pos) {
							if (*got_ip_pos == ',') {
								char *ip_pos = got_ip_pos + 1;
								if (ublox_chip)
									ip_pos++;
								uint8_t ip_len = (strstr(rxData, "####OK") - ip_pos) - 1;
								if (ublox_chip)
									ip_len--;
								uint8_t index = 0;

								for (index = 0; index < ip_len; index++) {
									if ((ip_pos[index] == '.')
											|| ((ip_pos[index] <= '9')
													&& (ip_pos[index] >= '0')))
										tmpBuf[index] = ip_pos[index];
								}
								tmpBuf[index] = '\0';
								strcpy(ip, tmpBuf);
								return true;
							} else {
								break;
							}
						}
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_ip] Error while getting module IP address!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::get_ip] Error: Timeout!\r\n");
					return false;
				}
			}

			if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd1, strlen(cmd1), 200)==HAL_OK) {
            	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS;
						cpt++) {
					if (SARAN21X_read_line(rxData, 64, false, 500)) {
						printf("%s\n", rxData);
						if (strstr(rxData, "##OK##") != NULL) {
							char* got_ip_pos = strstr(rxData, "+CGPADDR:") + strlen("+CGPADDR:");

							got_ip_pos++;
							if (ublox_chip)
								got_ip_pos++;

							if (got_ip_pos) {
								printf("'%s'\n", got_ip_pos);
								if (*got_ip_pos == ',') {
									char *ip_pos = got_ip_pos + 1;
									if (ublox_chip)
										ip_pos++;
									uint8_t ip_len = ((uint8_t)(strstr(rxData, "####OK") - ip_pos));
									if (ublox_chip)
										ip_len--;
									uint8_t index = 0;

									for (index = 0; index < ip_len; index++) {
										if ((ip_pos[index] == '.')
												|| ((ip_pos[index] <= '9')
														&& (ip_pos[index] >= '0')))
											tmpBuf[index] = ip_pos[index];
									}
									tmpBuf[index] = '\0';
									strcpy(ip, tmpBuf);
									return true;
								} else {
									printf(
											"[SARAN210::get_ip] module didn't get IP address!\r\n");
									return false;
								}

							}
						} else if (strstr(rxData, "##ERROR##") != NULL) {
							printf(
									"[SARAN210::get_ip] Error while getting module IP address!\r\n");
							return false;
						}
					}
					WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
					if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
						printf("[SARAN210::get_ip] Error: Timeout!\r\n");
						return false;
					}
				}
			}
		}
	}
	return false;
}

bool SARAN21X_get_cell_id(char* cell) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!cell) {
		printf("[SARAN210::get_cell_id] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+NUESTATS\r\n";
		char rxData[200];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 200, false, 1000)) {
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
						printf("cell_id %s \r\n", cell);
						return true;
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_cell_id] Error while getting module cell ID!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::get_cell_id] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_get_nuestats(char* cell, char* txpower) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!cell && !txpower) {
		printf("[SARAN210::get_nuestats] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+NUESTATS\r\n";
		char rxData[200];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 200, false, 1000)) {
					if (strstr(rxData, "##OK##") != NULL) {
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
							printf("cell_id %s \r\n", cell);

							//TX POWER
							char *tx_pos = strstr(rxData, "TX power")
									+ strlen("TX power") + 2;
							uint8_t tx_len = (uint8_t)(
									strstr(rxData, "##\"TX time") - tx_pos);
							for (index = 0; index < tx_len; index++) {
								txpower[index] = tx_pos[index];
							}
							txpower[index] = '\0';
							printf("txpower %s \r\n", txpower);
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
							printf("cell_id %s \r\n", cell);

							//TX POWER
							char *tx_pos = strstr(rxData, "TX power")
									+ strlen("TX power") + 1;
							uint8_t tx_len = (uint8_t)(
									strstr(rxData, "##TX time") - tx_pos);
							for (index = 0; index < tx_len; index++) {
								txpower[index] = tx_pos[index];
							}
							txpower[index] = '\0';
							printf("txpower %s \r\n", txpower);
							return true;
						}
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_nuestats] Error while getting module cell ID!\r\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::get_nuestats] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_get_RSSI(char* RSSI) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (!RSSI) {
		printf("[SARAN210::get_RSSI] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		const char* cmd = "AT+CSQ\r\n";
		char rxData[32];

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 32, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL) {
						if(ublox_chip){
							//RSSI
							char *RSSI_pos = strstr(rxData, "+CSQ:")
									+ strlen("+CSQ:") + 1;

							uint8_t index;
							//len of RSSI is always 2 chars
							for (index = 0; index < 2; index++) {
								RSSI[index] = RSSI_pos[index];
							}
							RSSI[index] = '\0';
							printf("rssi %s \r\n", RSSI);

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
							printf("rssi %s \r\n", RSSI);

							return true;

						}
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::get_RSSI] Error while getting module RSSI!\n");
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::get_RSSI] Error: Timeout!\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_open_socket(uint8_t* socket) {
	if (!socket) {
		printf("[SARAN210::open_socket] Error: Invalid input pointer!\r\n");
		return false;
	}

	if (mpUART) {
		char cmd[20];
		char rxData[16];

		if (ublox_chip)
			sprintf(cmd, "AT+NSOCR=\"DGRAM\",17,%d,1\r\n", (int) SARAN210_LOCAL_SEND_PORT);
		else
			sprintf(cmd, "AT+NSOCR=DGRAM,17,%d,1\r\n", (int) SARAN210_LOCAL_SEND_PORT);

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL) {
						for (uint8_t index = 0; index < strlen(rxData);
								index++) {
							if ((rxData[index] <= '9')
									&& (rxData[index] >= '0')) {
								*socket = rxData[index] - '0';
								return true;
							}
						}
						return false;
					} else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::open_socket] Error while opening socket on port %d!\r\n",
								(int) SARAN210_LOCAL_SEND_PORT);
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::open_socket] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_close_socket(uint8_t socket) {
	if (mpUART) {
		char cmd[20];
		char rxData[16];

		sprintf(cmd, "AT+NSOCL=%d\r\n", socket);

		if (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK) {
        	for (uint8_t cpt = 0; cpt < SARAN210_READ_UART_ATTEMPTS; cpt++) {
				if (SARAN21X_read_line(rxData, 16, false, 500)) {
					if (strstr(rxData, "##OK##") != NULL)
						return true;
					else if (strstr(rxData, "##ERROR##") != NULL) {
						printf(
								"[SARAN210::close_socket] Error while closing socket %d!\r\n",
								socket);
						return false;
					}
				}
				WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
				if (cpt >= (SARAN210_READ_UART_ATTEMPTS - 1)) {
					printf("[SARAN210::close_socket] Error: Timeout!\r\n");
					return false;
				}
			}
		}
	}
	return false;
}

bool SARAN21X_send(uint8_t* msg, uint8_t length) {
	WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
	if (mpUART) {
		if (!msg) {
			printf("[SARAN210::send] Error: Invalid input pointer!\r\n");
			return false;
		}

		if (!m_initialized) {
			printf("[SARAN210::send] Error: module not initialized!\r\n");
			return false;
		}
		/* Clear Rx buffer*/
		bool recstatus = SARAN21X_receive(SARAN210_BUFFER_SIZE);
		/*
		 cmd = "AT+NSOST=<socket>,\"<IP>\",<port>,<length>,\"msg\"\r\n"
		 cmd length = length
		 *          + 19 (strlen("AT+NSOST=,\"\",,,\"\"\r\n"))
		 *          + 3  (socket one byte: max value 255)
		 *          + 15 (IP: xxx.xxx.xxx.xxx)
		 *          + 5  (Port: max value 65535)
		 *          + 3  (Length field: max value 255)
		 *          + 1  ('\0')
		 *          = 46 + length
		 */
		char *cmd = (char *) malloc(46 + length);

		if (!cmd) {
			printf("[SARAN210::send] Error: cannot allocate memory!\r\n");
			return false;
		}
		if (ublox_chip)
			sprintf(cmd, "AT+NSOST=%d,\"%s\",%d,%d,\"", m_socket, m_server_ip, (int) m_server_port, length);
		else
			sprintf(cmd, "AT+NSOST=%d,%s,%d,%d,", m_socket, m_server_ip, (int) m_server_port, length);
		for (uint8_t cpt = 0; cpt < length; cpt++) {
			sprintf(cmd, "%s%02x", cmd, msg[cpt]);
		}

		if (ublox_chip)
			sprintf(cmd, "%s\"\r\n", cmd);
		else
			sprintf(cmd, "%s\r\n", cmd);

		printf("NB-IoT message sent with payload size %d to %s:%d, socket %d\r\n", length, m_server_ip, m_server_port, m_socket);
		
		bool result = (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, (uint16_t)strlen(cmd), 200)==HAL_OK);
        free(cmd);
		return result;
	}
	return false;
}

bool SARAN21X_receive(uint16_t length) {
	if (mpUART) {
		if (!m_initialized) {
			printf("[SARAN210::receive] Error: module not initialized!\r\n");
			return false;
		}
		char cmd[20];
		sprintf(cmd, "AT+NSORF=%d,%d\r\n", m_socket, length);
		return (HAL_UART_Transmit(uartHandle, (uint8_t *) cmd, strlen(cmd), 200)==HAL_OK);
	}
	return false;
}

uint16_t SARAN21X_read_line(char* ptr, uint16_t len, bool debug, uint32_t delay) {
	size_t index = 0;
	char tmpBuf[len];

	if (!ptr) {
		printf("[SARAN210::read_line] Error: Invalid input pointer!\r\n");
		return 0;
	}
	memset(tmpBuf, 0, len);
	memset(ptr, 0, len);

	if (!mpUART) {
		printf("[SARAN210::read_line] Error: Invalid UART object!\r\n");
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
		printf("read ");
		for (uint16_t cpt = 0; cpt < len; cpt++) {
			printf("%x\t", tmpBuf[cpt]);
		}
		printf("\nread %s\n", ptr);
	}
	return strlen(ptr);
}

uint64_t SARAN21X_convert_imsi_char_to_u64(const char* text)
{
	uint64_t number=0;

    for(; *text; text++)
    {
        char digit=*text-'0';           
        number=(number*10)+digit;
    }

    return number;
}