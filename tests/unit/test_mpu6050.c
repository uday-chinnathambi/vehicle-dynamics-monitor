/* tests/unit/test_mpu6050.c */
#include "unity.h"
#include "mpu6050.h"

void setUp(void) {}
void tearDown(void) {}

/* High=0x01 Low=0xF4 → 0x01F4 = 500 as signed int16 */
void test_raw_bytes_reconstruct_to_signed_int(void)
{
    int16_t result = reconstruct_raw_value(0x01U, 0xF4U);
    TEST_ASSERT_EQUAL_INT16(500, result);
}

/* 8192 raw counts / 8192 LSB/g = exactly 1.0g */
void test_raw_to_gforce_at_4g_range(void)
{
    float result = raw_to_gforce(8192, MPU6050_SENSITIVITY_4G);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, result);
}

/* Negative raw value must produce negative g-force: -4096 / 8192 = -0.5g */
void test_negative_raw_value_gives_negative_gforce(void)
{
    float result = raw_to_gforce(-4096, MPU6050_SENSITIVITY_4G);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.5f, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_raw_bytes_reconstruct_to_signed_int);
    RUN_TEST(test_raw_to_gforce_at_4g_range);
    RUN_TEST(test_negative_raw_value_gives_negative_gforce);
    return UNITY_END();
}
