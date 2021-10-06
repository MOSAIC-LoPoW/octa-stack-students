#include "payloadformat.h"
#include "datatypes.h"
#include "payloadparser.h"
#include "octa-protocol-types.h"

uint8_t parse_octa_configuration(struct octa_configuration* current_octa_configuration, uint8_t* buffer, uint16_t len)
{
  uint16_t index = 0;
  struct octa_configuration temp_config;
  //set temp config to use current config params
  temp_config = *current_octa_configuration;
  printDBG("parsing octa configuration payload with size %d\r\n", len);
  #if DEBUG
    for (uint16_t i = 0; i < len; ++i) {
      printf("%02x", buffer[i]);
    }
    printf("\r\n");
  #endif
  if(len<1)
  {
    printERR("Buffer too short\r\n");
    return PARSER_TOO_SHORT;
  }
  while (index<len)
  {
    switch(buffer[index])
    {
      case TYPE_DOWNLINK_ACK:
        if(len < index+5)
        {
          printERR("Data too short for ACK type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //ACK RECEIVED
        uint32LittleEndian.byte[0] = buffer[index+1];
        uint32LittleEndian.byte[1] = buffer[index+2];
        uint32LittleEndian.byte[2] = buffer[index+3];
        uint32LittleEndian.byte[3] = buffer[index+4];
        temp_config.last_msg_acked = uint32LittleEndian.integer;
        printINF("ACK for SEQ_NR: %d\r\n", temp_config.last_msg_acked);
        index += 5;
        break;
      case TYPE_DOWNLINK_INTERVAL_MULTIPLIER_UPDATE:
        if(len < index+3)
        {
          printERR("Data too short for INTERVAL_MULTIPLIER type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //INTERVAL & MULTIPLIER UPDATE RECEIVED
        temp_config.interval = buffer[index+1];
        temp_config.multiplier = buffer[index+2];
        printINF("New interval: %d and multiplier: %d\r\n", temp_config.interval, temp_config.multiplier);
        index += 3;
        break;
      case TYPE_DOWNLINK_DNS_UPDATE:
        if(len < index+2)
        {
          printERR("Data too short for DNS type.\r\n");
          return PARSER_PARSE_ERROR;
        } 
        //DNS ID UPDATE RECEIVED        
        if((buffer[index+1] != 1) && (buffer[index+1] != 0))
        {
          printERR("NON EXISTING DNS URL ID\r\n");
          return PARSER_INVALID_VALUE;
        }  
        else
        {
          temp_config.dns_id = buffer[index+1];
          printINF("New DNS URL id: %d\r\n", temp_config.dns_id);
          //TODO: reboot after new URL id? or just reinint nb-iot module? -> this is for application logic callbacks?
        }
        index += 2;
        break;
      case TYPE_CURRENT_FLASH_INDEX:
        if(len < index+5)
        {
          printERR("Data too short for FLASH type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //FLASH INDEX
        uint32LittleEndian.byte[0] = buffer[index+1];
        uint32LittleEndian.byte[1] = buffer[index+2];
        uint32LittleEndian.byte[2] = buffer[index+3];
        uint32LittleEndian.byte[3] = buffer[index+4];
        temp_config.flash_index = uint32LittleEndian.integer;
        printINF("Current Flash Index %d\r\n", temp_config.flash_index);
        index += 5;
        break;
      case TYPE_LAST_KNOWN_TIMESTAMP:
        if(len < index+5)
        {
          printERR("Data too short for LAST_KNOWN_TIMESTAMP type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //FLASH INDEX
        uint32LittleEndian.byte[0] = buffer[index+1];
        uint32LittleEndian.byte[1] = buffer[index+2];
        uint32LittleEndian.byte[2] = buffer[index+3];
        uint32LittleEndian.byte[3] = buffer[index+4];
        temp_config.last_known_timestamp = uint32LittleEndian.integer;
        printINF("Current last known timestamp %d\r\n", temp_config.last_known_timestamp);
        index += 5;
        break;
      default:
        printERR("UNKNOWN OCTA PAYLOAD TYPE RECEIVED\r\n");
        return PARSER_UNKNOWN_TYPE;
    }    
  }
  *current_octa_configuration = temp_config;
  return PARSER_OK;
}

uint8_t parse_octa_protocol_downlink(struct octa_configuration* current_octa_configuration, uint8_t* buffer, uint16_t len)
{
  uint16_t index = 0;
  struct octa_configuration temp_config;
  //set temp config to use current config params
  temp_config = *current_octa_configuration;
  printDBG("parsing octa protocol downlink with size %d\r\n", len);
  #if DEBUG
    for (uint16_t i = 0; i < len; ++i) {
      printf("%02x", buffer[i]);
    }
    printf("\r\n");
  #endif
  if(len<1)
  {
    printERR("Buffer too short\r\n");
    return PARSER_TOO_SHORT;
  }
  while (index<len)
  {
    switch(buffer[index])
    {
      case TYPE_HEADER_ACK:
        if(len < index+5)
        {
          printERR("Data too short for ACK type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //ACK RECEIVED
        uint32LittleEndian.byte[0] = buffer[index+1];
        uint32LittleEndian.byte[1] = buffer[index+2];
        uint32LittleEndian.byte[2] = buffer[index+3];
        uint32LittleEndian.byte[3] = buffer[index+4];
        temp_config.last_msg_acked = uint32LittleEndian.integer;
        printINF("ACK for SEQ_NR: %d\r\n", temp_config.last_msg_acked);
        index += 5;
        break;
      case TYPE_HEADER_END:
        printINF("Header of Payload END found\r\n");
        index ++;
        break;
      case TYPE_PAYLOAD_TIMESTAMP:
        if(len < index+5)
        {
          printERR("Data too short for TIMESTAMP type.\r\n");
          return PARSER_PARSE_ERROR;
        }
        //FLASH INDEX
        uint32LittleEndian.byte[0] = buffer[index+1];
        uint32LittleEndian.byte[1] = buffer[index+2];
        uint32LittleEndian.byte[2] = buffer[index+3];
        uint32LittleEndian.byte[3] = buffer[index+4];
        temp_config.last_known_timestamp = uint32LittleEndian.integer;
        RTC_Set_Time_From_Unix(temp_config.last_known_timestamp);
        printINF("UTC Timestamp received: %d\r\n", temp_config.last_known_timestamp);
        RTC_Print_DateTime();
        index += 5;
        break;
      default:
        printERR("UNKNOWN OCTA PAYLOAD TYPE RECEIVED\r\n");
        return PARSER_UNKNOWN_TYPE;
    }    
  }
  *current_octa_configuration = temp_config;
  printINF("[payloadparser:parse_octa_protocol_downlink] parsing OK\r\n");
  return PARSER_OK;
}

uint32_t parse_timestamp(uint8_t* buffer)
{
  uint32LittleEndian.byte[0] = buffer[0];
  uint32LittleEndian.byte[1] = buffer[1];
  uint32LittleEndian.byte[2] = buffer[2];
  uint32LittleEndian.byte[3] = buffer[3];
  return uint32LittleEndian.integer;
}