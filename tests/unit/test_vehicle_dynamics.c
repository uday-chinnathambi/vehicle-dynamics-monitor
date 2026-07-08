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

/* --- Boundary completeness ------------------------------------------------ */

/* ax at exactly +THRESHOLD_ACCEL_G → must trigger (boundary inclusive) */
void test_exact_threshold_acceleration(void)
{
    VehicleEvent result = evaluate_dynamics(THRESHOLD_ACCEL_G, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_RAPID_ACCELERATION, result);
}

/* ay at exactly +THRESHOLD_CORNERING_G → must trigger (boundary inclusive) */
void test_exact_threshold_cornering_left(void)
{
    VehicleEvent result = evaluate_dynamics(0.0f, THRESHOLD_CORNERING_G, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_CORNERING_LEFT, result);
}

/* ay at exactly -THRESHOLD_CORNERING_G → must trigger (boundary inclusive) */
void test_exact_threshold_cornering_right(void)
{
    VehicleEvent result = evaluate_dynamics(0.0f, -THRESHOLD_CORNERING_G, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_CORNERING_RIGHT, result);
}

/* ax just below +THRESHOLD_ACCEL_G → must NOT trigger */
void test_no_event_just_below_acceleration_threshold(void)
{
    VehicleEvent result = evaluate_dynamics(0.49f, 0.0f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

/* ay just below +THRESHOLD_CORNERING_G → must NOT trigger */
void test_no_event_just_below_cornering_threshold(void)
{
    VehicleEvent result = evaluate_dynamics(0.0f, 0.39f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

/* --- Simultaneous lateral + longitudinal events (priority order) ---------- */

/* Both braking and cornering-left exceed threshold: braking wins (highest priority) */
void test_braking_takes_priority_over_cornering_left(void)
{
    VehicleEvent result = evaluate_dynamics(-0.8f, 0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_HARSH_BRAKING, result);
}

/* Both braking and cornering-right exceed threshold: braking wins */
void test_braking_takes_priority_over_cornering_right(void)
{
    VehicleEvent result = evaluate_dynamics(-0.8f, -0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_HARSH_BRAKING, result);
}

/* Both acceleration and cornering-left exceed threshold: acceleration wins */
void test_acceleration_takes_priority_over_cornering_left(void)
{
    VehicleEvent result = evaluate_dynamics(0.8f, 0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_RAPID_ACCELERATION, result);
}

/* Both acceleration and cornering-right exceed threshold: acceleration wins */
void test_acceleration_takes_priority_over_cornering_right(void)
{
    VehicleEvent result = evaluate_dynamics(0.8f, -0.7f, 1.0f);
    TEST_ASSERT_EQUAL(EVENT_RAPID_ACCELERATION, result);
}

/* --- Debounce logic ------------------------------------------------------- */

/* A single spike must NOT immediately report an event */
void test_event_requires_hold_count(void)
{
    DebounceState state = {EVENT_NONE, 0U};
    VehicleEvent result = debounce_event(&state, EVENT_HARSH_BRAKING, DEBOUNCE_HOLD_COUNT);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

/* Event fires only after DEBOUNCE_HOLD_COUNT consecutive identical samples */
void test_event_fires_after_hold_count(void)
{
    DebounceState state = {EVENT_NONE, 0U};
    VehicleEvent result = EVENT_NONE;
    for (uint8_t i = 0U; i < DEBOUNCE_HOLD_COUNT; i++)
    {
        result = debounce_event(&state, EVENT_HARSH_BRAKING, DEBOUNCE_HOLD_COUNT);
    }
    TEST_ASSERT_EQUAL(EVENT_HARSH_BRAKING, result);
}

/* A different event mid-count resets the counter — caller gets EVENT_NONE */
void test_event_resets_on_different_input(void)
{
    DebounceState state = {EVENT_NONE, 0U};
    for (uint8_t i = 0U; i < DEBOUNCE_HOLD_COUNT - 1U; i++)
    {
        debounce_event(&state, EVENT_HARSH_BRAKING, DEBOUNCE_HOLD_COUNT);
    }
    VehicleEvent result = debounce_event(&state, EVENT_CORNERING_LEFT, DEBOUNCE_HOLD_COUNT);
    TEST_ASSERT_EQUAL(EVENT_NONE, result);
}

/* EVENT_NONE mid-count resets the counter; subsequent events need a full hold */
void test_none_resets_counter(void)
{
    DebounceState state = {EVENT_NONE, 0U};
    for (uint8_t i = 0U; i < DEBOUNCE_HOLD_COUNT - 1U; i++)
    {
        debounce_event(&state, EVENT_HARSH_BRAKING, DEBOUNCE_HOLD_COUNT);
    }
    debounce_event(&state, EVENT_NONE, DEBOUNCE_HOLD_COUNT);
    VehicleEvent result = debounce_event(&state, EVENT_HARSH_BRAKING, DEBOUNCE_HOLD_COUNT);
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
    /* boundary completeness */
    RUN_TEST(test_exact_threshold_acceleration);
    RUN_TEST(test_exact_threshold_cornering_left);
    RUN_TEST(test_exact_threshold_cornering_right);
    RUN_TEST(test_no_event_just_below_acceleration_threshold);
    RUN_TEST(test_no_event_just_below_cornering_threshold);
    /* simultaneous lateral + longitudinal priority */
    RUN_TEST(test_braking_takes_priority_over_cornering_left);
    RUN_TEST(test_braking_takes_priority_over_cornering_right);
    RUN_TEST(test_acceleration_takes_priority_over_cornering_left);
    RUN_TEST(test_acceleration_takes_priority_over_cornering_right);
    /* debounce logic */
    RUN_TEST(test_event_requires_hold_count);
    RUN_TEST(test_event_fires_after_hold_count);
    RUN_TEST(test_event_resets_on_different_input);
    RUN_TEST(test_none_resets_counter);
    UNITY_END();
}
