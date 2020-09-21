#include "stm32l4xx_hal.h"

typedef struct{
    uint16_t temp;
    int16_t volt;
    int32_t acc_capacity;
    int16_t current;
}bq35100_data_struct;

void bq35100_init(I2C_HandleTypeDef *hi2c);
void bq35100_read_acc_data(bq35100_data_struct* bq35100_data);