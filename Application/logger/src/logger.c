#include "logger.h"
#include "logger_cfg.h"
#include "hal_uart.h" // Assuming HAL_UART is the logging output
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"

/**
 * @file logger.c
 * @brief Implementation for the Logger component.
 *
 * This file contains the core logic for the logging module, including thread-safe
 * access, message formatting, and output.
 */

// --- Internal State Variables ---
static SemaphoreHandle_t s_log_mutex;
static char s_log_buffer[LOGGER_BUFFER_SIZE];
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static const char* LOGGER_GetLevelString(Log_Level_t level);

// --- Public Function Implementations ---

APP_Status_t LOGGER_Init(void) {
    if (s_is_initialized) {
        return APP_OK;
    }
    
    // Create a mutex for thread-safe access to the logging buffer and UART
    s_log_mutex = xSemaphoreCreateMutex();
    if (s_log_mutex == NULL) {
        // We can't log a failure here, as the logger is not yet initialized
        return APP_ERROR;
    }

    // Initialize the underlying output peripheral (e.g., UART)
    if (LOGGER_output_function_ptr(NULL, 0) != APP_OK) { // A special call to initialize the peripheral
        // We can't log this failure either
        return APP_ERROR;
    }

    s_is_initialized = true;
    LOGGER_SetLogLevel(LOGGER_DEFAULT_LOG_LEVEL);
    
    // Now we can log a success message
    LOGI("Logger", "Module initialized. Default log level is: %s", LOGGER_GetLevelString(LOGGER_DEFAULT_LOG_LEVEL));
    
    return APP_OK;
}

void LOGGER_SetLogLevel(Log_Level_t level) {
    if (level < LOG_LEVEL_COUNT) {
        LOGGER_current_log_level = level;
    }
}

void LOGGER_Log(Log_Level_t level, const char* module, int line, const char* format, ...) {
    if (!s_is_initialized || level < LOGGER_current_log_level || s_log_mutex == NULL) {
        return;
    }

    // Acquire the mutex to ensure thread-safe logging
    if (xSemaphoreTake(s_log_mutex, portMAX_DELAY) != pdTRUE) {
        // If we can't get the mutex, we can't log, so we just return.
        return;
    }

    int len;
    
    // Format the log message prefix
    len = snprintf(s_log_buffer, LOGGER_BUFFER_SIZE, "[%s] [%-15s:%-4d] ",
                   LOGGER_GetLevelString(level), module, line);
    
    if (len < 0 || len >= LOGGER_BUFFER_SIZE) {
        // Handle buffer overflow or error in snprintf
        strcpy(s_log_buffer, "[LOGGER_ERROR] ");
        len = 16;
    }
    
    // Append the user's message
    va_list args;
    va_start(args, format);
    int remaining_space = LOGGER_BUFFER_SIZE - len - 1; // -1 for null terminator
    int user_msg_len = vsnprintf(s_log_buffer + len, remaining_space, format, args);
    va_end(args);
    
    if (user_msg_len < 0 || (len + user_msg_len) >= LOGGER_BUFFER_SIZE) {
        // The formatted message was too long, terminate it and append an ellipsis
        strcpy(s_log_buffer + LOGGER_BUFFER_SIZE - 4, "...\n");
        len = LOGGER_BUFFER_SIZE;
    } else {
        len += user_msg_len;
        // Append a newline if it's not already there
        if (len == 0 || s_log_buffer[len-1] != '\n') {
            s_log_buffer[len] = '\n';
            len++;
        }
    }
    
    // Output the final log string
    LOGGER_output_function_ptr(s_log_buffer, len);
    
    // Release the mutex
    xSemaphoreGive(s_log_mutex);
}

// --- Private Helper Function Implementations ---

static const char* LOGGER_GetLevelString(Log_Level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO ";
        case LOG_LEVEL_WARNING: return "WARN ";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}