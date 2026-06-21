#include "vehicle_dynamics.h"

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