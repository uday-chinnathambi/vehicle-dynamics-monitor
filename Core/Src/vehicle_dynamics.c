/**
 * @file  vehicle_dynamics.c
 * @brief Application logic layer — implements evaluate_dynamics() and debounce_event().
 *        No HAL calls; all logic operates on floats and enums only.
 */
#include "vehicle_dynamics.h"

VehicleEvent debounce_event(DebounceState *state, VehicleEvent raw_event, uint8_t hold_required)
{
    if (raw_event != state->candidate)
    {
        state->candidate = raw_event;
        state->count = 0U;
    }

    if (state->count < hold_required)
    {
        state->count++;
    }

    return (state->count >= hold_required) ? state->candidate : EVENT_NONE;
}

VehicleEvent evaluate_dynamics(float longitudinal_g, float lateral_g, float vertical_g)
{
    VehicleEvent result = EVENT_NONE;

    (void)vertical_g; /* reserved for future bump/pothole detection */

    if (longitudinal_g <= -THRESHOLD_BRAKING_G)
    {
        result = EVENT_HARSH_BRAKING;
    }
    else if (longitudinal_g >= THRESHOLD_ACCEL_G)
    {
        result = EVENT_RAPID_ACCELERATION;
    }
    else if (lateral_g >= THRESHOLD_CORNERING_G)
    {
        result = EVENT_CORNERING_LEFT;
    }
    else if (lateral_g <= -THRESHOLD_CORNERING_G)
    {
        result = EVENT_CORNERING_RIGHT;
    }
    else
    {
        /* all axes within limits — no event */
    }

    return result;
}