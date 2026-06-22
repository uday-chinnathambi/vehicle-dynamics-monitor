#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

/* Sensitivity scale factor: LSB counts per 1g at ±4g full-scale range (MPU-6050 datasheet) */
#define MPU6050_SENSITIVITY_4G  (8192U)

/*
 * Reconstructs a signed 16-bit raw sensor value from two bytes received over I2C.
 * The MPU-6050 sends the high byte first, followed by the low byte.
 *
 * high_byte : most-significant byte of the raw axis register pair
 * low_byte  : least-significant byte of the raw axis register pair
 *
 * Returns the signed 16-bit two's-complement value.
 */
int16_t reconstruct_raw_value(uint8_t high_byte, uint8_t low_byte);

/*
 * Converts a raw 16-bit sensor count to a g-force value.
 *
 * raw_value            : signed raw count from reconstruct_raw_value()
 * sensitivity_lsb_per_g: scale factor for the configured full-scale range
 *                        (use MPU6050_SENSITIVITY_4G for ±4g)
 *
 * Returns acceleration in g-force as a float.
 */
float raw_to_gforce(int16_t raw_value, uint16_t sensitivity_lsb_per_g);

#endif /* MPU6050_H */
