#include "dns.h"
#include "platform.h"
#include "string.h" //for strlen
#include "stdlib.h" //for malloc
#include "stdio.h" //for sprintf

uint8_t* generateRequest(char *host, uint16_t* length)
{
	*length = strlen(host) + 4 + DNS_HEADER_LENGTH + 2; // 1 extra byte in front and after the host string; 4 bytes to addt type and class
	uint8_t* buf = malloc(*length);
	
	buf[0] = 0x12;	// ID (part 1)
	buf[1] = 0x34;	// ID (part 2)
	buf[2] = 0x01;	// Recursion desired
	buf[3] = 0x00;	// flags
	buf[4] = 0x00;	// Questions (part 1)
	buf[5] = 0x01;	// Questions (part 2)
	buf[6] = 0x00;	// Answers (part 1)
	buf[7] = 0x00;	// Answers (part 2)
	buf[8] = 0x00;	// Authority RR (part 1)
	buf[9] = 0x00;	// Authority RR (part 2)
	buf[10] = 0x00;	// Additional RR (part 1)
	buf[11] = 0x00;	// Additional RR (part 2)

	int i = DNS_HEADER_LENGTH;
	int count = 0;
	int len_pos;
	
	while (i < DNS_HEADER_LENGTH + strlen(host)) {
		len_pos = i;
		count = 0;

		while (host[i - DNS_HEADER_LENGTH] && host[i - DNS_HEADER_LENGTH] != '.') {
			buf[i + 1] = host[i - DNS_HEADER_LENGTH];
			++i;
			++count;
		}
		buf[len_pos] = count;
		++i;
	}

	buf[i] = 0x00;
	buf[i+1] = 0x00;
	buf[i+2] = 0x01;	// A record
	buf[i+3] = 0x00;
	buf[i+4] = 0x01;	// Class: IN

	return buf;
}

char* parseReply(uint8_t* reply, const uint16_t length) {
	char* address = malloc(16);
	
	int i = 12;
	int j = 0;

	if (length < 4 || reply[3] & 15) {
		// DNS response did not contain a valid answer
		return 0;
	}

	while (i < length) {
		// Jump over the query to the answer
		int jump = reply[i];
		
		if (jump) {
			i += jump + 1;
			} else {
				// jump == 0, so that is the mark of the end of the request URL.
				// 8 bytes further, we find  is the first reply type.
				// Now, first check if this answer is a CNAME answer (0x05) or an A record (0x01)
				i += 8;

				while ( i < length) {
					if (reply[i] == 0x01) {
						printDBG("The DNS reply contains an A record.\r\n");
						// The  answer is an A record. We will use it to construct the IP address.
						// Advance 9 bytes (which is the start of the IP address in the answer).
						i +=9;

						for (int k = 0; k < 4; ++k) {
							if (i < length) {
								uint8_t buf[4] = { 0, 0, 0, 0 };
								int l = 0;

								sprintf(buf, "%d", reply[i++]);
							
								// Copy the buffer to the address array
								while (buf[l]) {
									address[j++] = buf[l++];
								}
							
								if (k < 3){
									// Add the '.' in the IP address
									address[j++] = '.';
								} else {
									i = length;
								}
							}
						}
						break;
					} else if (reply[i] == 0x05) {
						// The answer is a CNAME. we will skip over it
						printDBG("The DNS reply contains an CNAME record.\r\n");

						// At bytes 7 and 8, the length of the remainder of the answer is stored.
						uint16_t cname_length = 0;
						cname_length += reply[i + 7];
						cname_length = cname_length << 8;
						cname_length |= reply[i + 8];
						// Advance i with 8 (length byte position) + length of the rest of the reply + 4 (next type field)
						i += 8 + cname_length + 4;
					} else {
						printDBG("Unknown DNS answer type '%x' received in reply. Aborting\r\n", reply[i]);
						return 0;
					}
				}
			}				
        }

		// If. somehow, we did not fill in an IP address, return 0.
		if (j == 0) {
			return 0;
		}

		// Fill up the rest of the address array with 0x00.
        for (int c = j; c < 16; ++c) {
                address[c] = 0;
        }

        return address;
}