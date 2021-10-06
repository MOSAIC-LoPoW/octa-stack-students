/*
1. Message structure
An OCTA message consists of two parts, the header containing metadata about the device and the payload containing data measured by sensors connected to it.
Header types are defined in section 2, the header is ended by using the special header type FF. The END header type can be omitted if the message does not contain any payload.
Below, an example of an OCTA message containing both a header and payload is shown.

H1 xx xx H2 xx ... FF P1 xx xx P2 xx xx xx xx...
*/

/* HEADER TYPES */
#define TYPE_HEADER_APPLICATION_VERSION                 0x00
#define TYPE_HEADER_STACK_VERSION                       0x01
#define TYPE_HEADER_PROTOCOL_VERSION                    0x02
#define TYPE_HEADER_OCTA_ID                             0x03
#define TYPE_HEADER_OCTA_STATUS                         0x04
#define TYPE_HEADER_TECH                                0x05
        #define HEADER_TECH_NBIOT                       0x00
#define TYPE_HEADER_SEQ_NR                              0x06
#define TYPE_HEADER_ACK                                 0x07

#define TYPE_HEADER_END                                 0xFF

/* PAYLOAD TYPES */
#define TYPE_PAYLOAD_TIMESTAMP                          0x00
#define TYPE_PAYLOAD_BLOB                               0x01
#define TYPE_PAYLOAD_CO2                                0x02
#define TYPE_PAYLOAD_RH                                 0x03
#define TYPE_PAYLOAD_TEMPERATURE                        0x04
#define TYPE_PAYLOAD_BATTERY_VOLTAGE                    0x05

#define TYPE_PAYLOAD_END                                0xFF

/* APPLICATION ID's */
#define APPLICATION_ID_OCTA_PROTOCOL_TESTER             0
#define APPLICATION_ID_IOW_WQMD                         1
#define APPLICATION_ID_BEACON_CO2                       2     