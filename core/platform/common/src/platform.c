#include "platform.h"

void Initialize_Platform(void)
{
    #ifdef platform_octa
        OCTA_Initialize_Platform();
    #endif
    #ifdef platform_octa_mini
        printf("octa mini not yet implemented \r\n");
    #endif
}
