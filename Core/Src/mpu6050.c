/**
 * @file  mpu6050.c
 * @brief Sensor HAL layer — MPU-6050 register access over I2C and axis data conversion.
 */
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

/* Sets ACCEL_CONFIG register: AFS_SEL bits [4:3] = 01 → ±4g range */
HAL_StatusTypeDef mpu6050_configure_accel_range(I2C_HandleTypeDef *hi2c)
{
    return mpu6050_write_register(hi2c, MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_FS_4G);
}

/* Sets SMPLRT_DIV register: sample rate = 8000 / (1 + divider) Hz */
HAL_StatusTypeDef mpu6050_configure_sample_rate(I2C_HandleTypeDef *hi2c, uint8_t divider)
{
    return mpu6050_write_register(hi2c, MPU6050_REG_SMPLRT_DIV, divider);
}

/* Burst-reads 6 raw accelerometer bytes starting at ACCEL_XOUT_H (0x3B) */
HAL_StatusTypeDef mpu6050_read_accel_raw(I2C_HandleTypeDef *hi2c,
                                          uint8_t *buf, uint16_t buf_size)
{
    if (buf_size < MPU6050_ACCEL_RAW_BYTES)
    {
        return HAL_ERROR;
    }
    return HAL_I2C_Mem_Read(hi2c, MPU6050_I2C_ADDR,
                            MPU6050_REG_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT,
                            buf, MPU6050_ACCEL_RAW_BYTES, HAL_MAX_DELAY);
}

#endif /* BUILD_TESTS */

