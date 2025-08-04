#include "logger_cfg.h"
#include "hal_uart.h"

/**
 * @file logger_cfg.c
 * @brief Configuration data definitions for the Logger component.
 *
 * This file provides the definitions for the configurable parameters of the
 * Logger module.
 */

// --- Global Variable Definitions ---
Log_Level_t LOGGER_current_log_level = LOGGER_DEFAULT_LOG_LEVEL;

// Define the output function pointer. This should point to the low-level
// driver function that sends data to the desired output peripheral.
APP_Status_t LOGGER_uart_output_function(const char* data, uint16_t length) {
    // This is a wrapper function for the actual HAL driver
    if (data == NULL && length == 0) {
        // Special case for initialization
        return HAL_UART_Init(HAL_UART_UNIT_0, 115200);
    }
    return HAL_UART_Transmit(HAL_UART_UNIT_0, (uint8_t*)data, length, 100);
}

LOGGER_OutputFunction_t LOGGER_output_function_ptr = LOGGER_uart_output_function;