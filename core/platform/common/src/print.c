#include "print.h"

void printINF(char *format, ...)
{
    printf("[INF] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void printERR(char *format, ...)
{
    printf("[ERR] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void printDBG(char *format, ...)
{
    #if DEBUG
        printf("[DBG] ");
        // https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/vprintf.htm
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    #endif
}