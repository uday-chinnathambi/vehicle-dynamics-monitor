/**
 * @file  uart_logger.c
 * @brief Output layer — snprintf formatting and HAL_UART_Transmit for g-force and events.
 */
#include "uart_logger.h"

#ifndef BUILD_TESTS
#include <string.h>
#include <stdio.h>

static const char * const EVENT_NAMES[] = {
    "",
    "HARSH_BRAKING",
    "RAPID_ACCELERATION",
    "CORNERING_LEFT",
    "CORNERING_RIGHT"
};

void uart_log_accel(UART_HandleTypeDef *huart, float x_g, float y_g, float z_g)
{
    char buf[48];
    snprintf(buf, sizeof(buf), "X:%+.3fg  Y:%+.3fg  Z:%+.3fg\r\n", x_g, y_g, z_g);
    HAL_UART_Transmit(huart, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
}

void uart_log_event(UART_HandleTypeDef *huart, VehicleEvent event)
{
    if (event == EVENT_NONE)
    {
        return;
    }
    char buf[40];
    snprintf(buf, sizeof(buf), "EVENT: %s\r\n", EVENT_NAMES[event]);
    HAL_UART_Transmit(huart, (uint8_t *)buf, strlen(buf), HAL_MAX_DELAY);
}

#endif /* BUILD_TESTS */
