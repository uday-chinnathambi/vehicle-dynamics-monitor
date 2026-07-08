#ifndef VEHICLE_DYNAMICS_H
#define VEHICLE_DYNAMICS_H

#include <stdint.h>

/* Thresholds in g-force units (MISRA Rule 20.7: parenthesise macro values) */
#define THRESHOLD_BRAKING_G     (0.5f)
#define THRESHOLD_ACCEL_G       (0.5f)
#define THRESHOLD_CORNERING_G   (0.4f)

typedef enum
{
    EVENT_NONE = 0,
    EVENT_HARSH_BRAKING,
    EVENT_RAPID_ACCELERATION,
    EVENT_CORNERING_LEFT,
    EVENT_CORNERING_RIGHT
} VehicleEvent;

/*
 * Classifies a driving event from a single sample of acceleration data.
 *
 * longitudinal_g : forward/backward g-force  (+ve = accelerating, -ve = braking)
 * lateral_g      : left/right g-force         (+ve = left turn,   -ve = right turn)
 * vertical_g     : vertical g-force            (nominally +1.0g at rest on flat ground)
 *
 * Returns the detected VehicleEvent, or EVENT_NONE if all axes are within limits.
 * Priority order: braking > acceleration > cornering left > cornering right.
 */
VehicleEvent evaluate_dynamics(float longitudinal_g, float lateral_g, float vertical_g);

/* Minimum number of consecutive identical events required before reporting */
#define DEBOUNCE_HOLD_COUNT (3U)

/* Persistent state for debounce_event — zero-initialise before first call */
typedef struct
{
    VehicleEvent candidate;
    uint8_t      count;
} DebounceState;

/*
 * Suppresses transient spikes by requiring hold_required consecutive identical
 * raw events before the event is forwarded to the caller.
 *
 * state        : persistent debounce state; must be zero-initialised by the caller
 * raw_event    : output of evaluate_dynamics() for the current sample
 * hold_required: consecutive-sample threshold (use DEBOUNCE_HOLD_COUNT)
 *
 * Returns raw_event once it has been stable for hold_required samples,
 * otherwise returns EVENT_NONE.
 */
VehicleEvent debounce_event(DebounceState *state, VehicleEvent raw_event, uint8_t hold_required);

#endif /* VEHICLE_DYNAMICS_H */
