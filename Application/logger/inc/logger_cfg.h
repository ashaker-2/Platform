#ifndef LOGGER_CFG_H
#define LOGGER_CFG_H

#include "logger.h"
#include "common.h"

/**
 * @file logger_cfg.h
 * @brief Configuration header for the Logger component.
 *
 * This file defines the configuration macros and external declarations for
 * the Logger module.
 */

// --- Configuration Macros ---
// #define LOGGER_DEFAULT_LOG_LEVEL        LOG_LEVEL_INFO
#define LOGGER_BUFFER_SIZE              256

// --- External Declarations ---
// The current log level is defined in logger_cfg.c and modified by LOGGER_SetLogLevel
extern Log_Level_t LOGGER_current_log_level;

// The output function pointer is defined in logger_cfg.c and points to the
// actual HAL output function (e.g., HAL_UART_Transmit).
typedef Status_t (*LOGGER_OutputFunction_t)(const char* data, uint16_t length);
extern LOGGER_OutputFunction_t LOGGER_output_function_ptr;

#endif // LOGGER_CFG_H