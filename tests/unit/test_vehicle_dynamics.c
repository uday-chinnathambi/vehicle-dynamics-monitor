/* tests/unit/test_vehicle_dynamics.c */
#include "unity.h"
#include "vehicle_dynamics.h"

void setUp(void) {}
void tearDown(void) {}

/* All axes well within limits — no event expected */
void test_no_event_when_accel_within_limits(void)
{
    VehicleEvent result = evaluate_dynamics(0.1f, 0.05f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

/* ax well below -THRESHOLD_BRAKING_G → harsh braking */
void test_harsh_braking_detected(void)
{
    VehicleEvent result = evaluate_dynamics(-0.8f, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_HARSH_BRAKING, result);
}

/* ax well above +THRESHOLD_ACCEL_G → rapid acceleration */
void test_rapid_acceleration_detected(void)
{
    VehicleEvent result = evaluate_dynamics(0.8f, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_RAPID_ACCELERATION, result);
}

/* ay well above +THRESHOLD_CORNERING_G → left cornering */
void test_aggressive_cornering_left(void)
{
    VehicleEvent result = evaluate_dynamics(0.0f, 0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_CORNERING_LEFT, result);
}

/* ax at exactly -THRESHOLD_BRAKING_G → must trigger (boundary inclusive) */
void test_event_at_exact_threshold(void)
{
    VehicleEvent result = evaluate_dynamics(-THRESHOLD_BRAKING_G, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_HARSH_BRAKING, result);
}

/* ay well below -THRESHOLD_CORNERING_G → right cornering */
void test_aggressive_cornering_right(void)
{
    VehicleEvent result = evaluate_dynamics(0.0f, -0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_CORNERING_RIGHT, result);
}

/* ax just below threshold must NOT trigger */
void test_no_event_just_below_braking_threshold(void)
{
    VehicleEvent result = evaluate_dynamics(-0.49f, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_no_event_when_accel_within_limits);
    RUN_TEST(test_harsh_braking_detected);
    RUN_TEST(test_rapid_acceleration_detected);
    RUN_TEST(test_aggressive_cornering_left);
    RUN_TEST(test_aggressive_cornering_right);
    RUN_TEST(test_event_at_exact_threshold);
    RUN_TEST(test_no_event_just_below_braking_threshold);
    UNITY_END();
}
