#ifndef UART_LOGGER_H
#define UART_LOGGER_H

#include "vehicle_dynamics.h"

#ifndef BUILD_TESTS
#include "stm32f4xx_hal.h"

/*
 * Transmits a formatted g-force reading over UART.
 * Format: "X:+1.234g  Y:+1.234g  Z:+1.234g\r\n"
 */
void uart_log_accel(UART_HandleTypeDef *huart, float x_g, float y_g, float z_g);

/*
 * Transmits a human-readable event name over UART.
 * Does nothing if event is EVENT_NONE.
 * Format: "EVENT: HARSH_BRAKING\r\n"
 */
void uart_log_event(UART_HandleTypeDef *huart, VehicleEvent event);

#endif /* BUILD_TESTS */

#endif /* UART_LOGGER_H */
