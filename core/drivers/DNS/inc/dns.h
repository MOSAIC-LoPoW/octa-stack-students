#include <stdint.h>

//Function Declarations
uint8_t* generateRequest(char* host, uint16_t* length);
char* parseReply(uint8_t* reply, const uint16_t length);

#define DNS_HEADER_LENGTH 12