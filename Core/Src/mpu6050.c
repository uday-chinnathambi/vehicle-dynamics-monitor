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

#ifndef BUILD_TESTS

HAL_StatusTypeDef mpu6050_read_register(I2C_HandleTypeDef *hi2c,
                                         uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(hi2c, MPU6050_I2C_ADDR,
                            reg, I2C_MEMADD_SIZE_8BIT,
                            value, 1U, HAL_MAX_DELAY);
}

HAL_StatusTypeDef mpu6050_write_register(I2C_HandleTypeDef *hi2c,
                                          uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(hi2c, MPU6050_I2C_ADDR,
                             reg, I2C_MEMADD_SIZE_8BIT,
                             &value, 1U, HAL_MAX_DELAY);
}

/* Clears PWR_MGMT_1 — brings sensor out of sleep mode */
HAL_StatusTypeDef mpu6050_wake(I2C_HandleTypeDef *hi2c)
{
    return mpu6050_write_register(hi2c, MPU6050_REG_PWR_MGMT_1, 0x00U);
}

/* Returns HAL_OK only if WHO_AM_I reads back 0x68 */
HAL_StatusTypeDef mpu6050_verify_identity(I2C_HandleTypeDef *hi2c)
{
    uint8_t who_am_i = 0U;
    HAL_StatusTypeDef status = mpu6050_read_register(hi2c,
                                   MPU6050_REG_WHO_AM_I, &who_am_i);
    if ((status == HAL_OK) && (who_am_i != MPU6050_WHO_AM_I_VALUE))
    {
        status = HAL_ERROR;
    }
    return status;
}

#endif /* BUILD_TESTS */

