#include <stdint.h>

#define PARSER_OK               0
#define PARSER_TOO_SHORT        1
#define PARSER_PARSE_ERROR      2
#define PARSER_UNKNOWN_TYPE     3
#define PARSER_INVALID_VALUE    4

// Configuration has a fixed size (in flash) because it has a fixed structure
#define CONFIG_BUFFER_SIZE        15

uint8_t parse_octa_configuration(struct octa_configuration* current_octa_configuration, uint8_t* buffer, uint16_t len);