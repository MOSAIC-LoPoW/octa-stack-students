#include "stdint.h"

#define assert_param(expr) ((expr) ? (void)0U : assert_failed((char *)__FILE__, __LINE__))
#define assert(x) assert_param(x)

void assert_failed(char *FILE, uint32_t line );