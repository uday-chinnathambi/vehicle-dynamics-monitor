#include "mpu6050.h"

int16_t reconstruct_raw_value(uint8_t high_byte, uint8_t low_byte)
{
    int16_t result = (int16_t)(((uint16_t)high_byte << 8U) | (uint16_t)low_byte);
    return result;
}

float raw_to_gforce(int16_t raw_value, uint16_t sensitivity_lsb_per_g)
{
    float gforce_val = (float)raw_value / (float)sensitivity_lsb_per_g;
    return gforce_val;
}
