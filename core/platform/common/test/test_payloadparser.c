#include "unity.h"
#include "datatypes.h"
#include "print.h"
#include "payloadparser.h"

struct octa_configuration test_octa_configuration = {0};
uint8_t buffer[20];
uint16_t len;

void setUp(void) 
{
    test_octa_configuration.flash_index = 0;
    test_octa_configuration.dns_id = 0;
    test_octa_configuration.last_msg_acked = 0;
    test_octa_configuration.interval = 0;
    test_octa_configuration.multiplier = 0;
}

void tearDown(void) {} // every test file requires this function;
                       // tearDown() is called by the generated runner before each test case function

void test_payloadparser_parseOctaConfiguration_should_ReturnCONFIG_UNKNOWN_TYPEWhenUnknownTypeIsPassed(void)
{
    buffer[0] = 1;
    buffer[1] = 2; 
    len = 2;
    TEST_ASSERT_EQUAL(PARSER_UNKNOWN_TYPE, parse_octa_configuration(&test_octa_configuration, buffer, len));
}

void test_payloadparser_parseOctaConfiguration_should_UpdateConfigurationStructWhenValidConfigIsParsed(void)
{
    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 3;
    buffer[4] = 4;
    //interval & multiplier
    buffer[5] = 246;
    buffer[6] = 1;
    buffer[7] = 5;
    //dns_id
    buffer[8] = 245;
    buffer[9] = 1;
    //flash index
    buffer[10] = 38;
    buffer[11] = 23;
    buffer[12] = 0;
    buffer[13] = 0;
    buffer[14] = 0;

    len = 15;
    struct octa_configuration wanted_octa_configuration = { .flash_index = 23, 
                                                            .dns_id = 1, 
                                                            .last_msg_acked = 67305985, 
                                                            .interval = 1, 
                                                            .multiplier = 5};

    TEST_ASSERT_EQUAL(PARSER_OK, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);

    //update msg acked to 1
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    len = 5;
    wanted_octa_configuration.last_msg_acked = 1;

    TEST_ASSERT_EQUAL(PARSER_OK, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);

    //dns_id
    buffer[0] = 245;
    buffer[1] = 0;
    len = 2;
    wanted_octa_configuration.dns_id = 0;

    TEST_ASSERT_EQUAL(PARSER_OK, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);
}

void test_payloadparser_parseOctaConfiguration_should_ReturnCONFIG_TOO_SHORTWhenEmptyBufferIsPassed(void)
{
    len = 0;
    TEST_ASSERT_EQUAL(PARSER_TOO_SHORT, parse_octa_configuration(&test_octa_configuration, buffer, len));
}

void test_payloadparser_parseOctaConfiguration_should_ReturnCONFIG_INVALID_VALUEWhenUnkownDNSIdIsParsed(void)
{    
    //dns acked
    buffer[0] = 245;
    //2 does not exist
    buffer[1] = 2;
    len = 2;

    TEST_ASSERT_EQUAL(PARSER_INVALID_VALUE, parse_octa_configuration(&test_octa_configuration, buffer, len));
}

void test_payloadparser_parseOctaConfiguration_should_ReturnCONFIG_PARSER_ERROROnUnderSizedMessage(void)
{
    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 2;

    len = 4;
    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));

    //interval acked
    buffer[0] = 246;
    buffer[1] = 1;

    len = 2;
    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));

    //dns acked
    buffer[0] = 245;

    len = 1;
    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));

    //dns acked
    buffer[0] = 38;
    buffer[1] = 1;
    buffer[2] = 1;
    buffer[3] = 1;

    len = 4;
    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));
}

void test_payloadparser_parseOctaConfiguration_should_ReturnCONFIG_PARSER_UNKNOWN_TYPEOnOverSizedMessage(void)
{
    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 2;
    buffer[4] = 2;
    buffer[5] = 2;

    len = 6;
    TEST_ASSERT_EQUAL(PARSER_UNKNOWN_TYPE, parse_octa_configuration(&test_octa_configuration, buffer, len));
}

void test_payloadparser_parseOctaConfiguration_should_NotUpdateConfigWheneverAnErrorOccursDuringParsing(void)
{
    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 3;
    buffer[4] = 4;
    //interval & multiplier
    buffer[5] = 246;
    buffer[6] = 1;
    buffer[7] = 5;
    //dns_id
    buffer[8] = 245;
    buffer[9] = 1;

    //flash index -> too short, should drop everything
    buffer[10] = 38;
    buffer[11] = 23;
    buffer[12] = 0;
    len = 13;

    //should be 0, everything dropped
    struct octa_configuration wanted_octa_configuration = { 0 };

    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);
}

void test_payloadparser_parseOctaConfiguration_should_ParseCorrectlyAfterError(void)
{
    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 3;
    buffer[4] = 4;
    //interval & multiplier
    buffer[5] = 246;
    buffer[6] = 1;
    buffer[7] = 5;
    //dns_id
    buffer[8] = 245;
    buffer[9] = 1;

    //flash index -> too short, should drop everything
    buffer[10] = 38;
    buffer[11] = 23;
    buffer[12] = 0;
    len = 13;

    //should be 0, everything dropped
    struct octa_configuration wanted_octa_configuration = { 0 };

    TEST_ASSERT_EQUAL(PARSER_PARSE_ERROR, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);

    //msg acked
    buffer[0] = 247;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 3;
    buffer[4] = 4;
    //interval & multiplier
    buffer[5] = 246;
    buffer[6] = 1;
    buffer[7] = 2;

    len = 8;

    //should be 0, everything dropped
    wanted_octa_configuration.last_msg_acked= 67305985;
    wanted_octa_configuration.interval = 1;
    wanted_octa_configuration.multiplier = 2;

    TEST_ASSERT_EQUAL(PARSER_OK, parse_octa_configuration(&test_octa_configuration, buffer, len));

    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.flash_index, test_octa_configuration.flash_index);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.dns_id, test_octa_configuration.dns_id);
    TEST_ASSERT_EQUAL_INT32(wanted_octa_configuration.last_msg_acked, test_octa_configuration.last_msg_acked);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.interval, test_octa_configuration.interval);
    TEST_ASSERT_EQUAL_UINT8(wanted_octa_configuration.multiplier, test_octa_configuration.multiplier);

}