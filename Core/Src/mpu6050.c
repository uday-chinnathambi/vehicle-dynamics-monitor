#include "mpu6050.h"

int16_t reconstruct_raw_value(uint8_t high_byte, uint8_t low_byte)
{
    (void)high_byte;
    (void)low_byte;
    return 0;
}

float raw_to_gforce(int16_t raw_value, uint16_t sensitivity_lsb_per_g)
{
    (void)raw_value;
    (void)sensitivity_lsb_per_g;
    return 0.0f;
}
