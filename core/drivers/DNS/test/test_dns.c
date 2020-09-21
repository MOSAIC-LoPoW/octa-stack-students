#include "unity.h"
#include "dns.h"
// why do i need print here?
#include "print.h"

void setUp(void) {}    // every test file requires this function;
                       // setUp() is called by the generated runner before each test case function

void tearDown(void) {} // every test file requires this function;
                       // tearDown() is called by the generated runner before each test case function

void test_dns_parseReply_should_ReturnZeroOnInvalidReply(void)
{
    uint8_t reply[2] =  {0x01, 0x02};
    uint16_t len = 2;
    TEST_ASSERT_EQUAL(0, parseReply(reply, len));
}