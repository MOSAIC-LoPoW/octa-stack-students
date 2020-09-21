#include <stdint.h> 

union{
    int32_t integer;
    uint8_t byte[4];
} int32LittleEndian;

union{
    uint32_t integer;
    uint8_t byte[4];
} uint32LittleEndian;

union{
    int16_t integer;
    uint8_t byte[2];
} int16LittleEndian;

union{
    uint16_t integer;
    uint8_t byte[2];
} uint16LittleEndian;

union {
    float fl;
    struct
    {
        uint8_t b1, b2, b3, b4;
    } bytes;
} float_union;

union {
    uint64_t u64;
    struct
    {
        uint8_t b1, b2, b3, b4, b5, b6, b7, b8;
    } bytes;
} octa_uid;

struct octa_configuration{
    uint32_t flash_index;
    uint8_t dns_id;
    int32_t last_msg_acked;
    uint8_t interval;
    uint8_t multiplier;
};
