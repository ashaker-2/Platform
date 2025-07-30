// app/src/logger.c

#include "logger.h"
#include "logger_config.h"
#include "ecual_uart.h"     // For serial output
#include "ecual_common.h"   // For ECUAL_GetUptimeMs()

#include <stdio.h>    // For snprintf, vsnprintf
#include <string.h>   // For strlen
#include <stdarg.h>   // For va_list, va_start, va_end

// Max log line length (including timestamp, level, tag, message, newline, null terminator)
// Adjust this based on your needs and available stack/heap space.
#define MAX_LOG_LINE_LENGTH 256

static bool is_initialized = false;
static char log_line_buffer[MAX_LOG_LINE_LENGTH]; // Buffer for single log line

// Function to convert LOG_LEVEL_T to a short string
static const char* get_log_level_str(LOG_LEVEL_T level) {
    switch (level) {
        case LOG_ERROR:   return "E";
        case LOG_WARN:    return "W";
        case LOG_INFO:    return "I";
        case LOG_DEBUG:   return "D";
        case LOG_VERBOSE: return "V";
        case LOG_NONE:    return "N"; // Should not be logged
        default:              return "?"; // Unknown
    }
}

// Helper to get current timestamp string (e.g., "[HH:MM:SS.ms]")
static void get_timestamp_str(char *buf, size_t buf_len) {
    uint64_t uptime_ms = ECUAL_GetUptimeMs();
    uint32_t seconds = uptime_ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;

    snprintf(buf, buf_len, "[%02u:%02u:%02u.%03u]", (unsigned int)hours, (unsigned int)minutes, (unsigned int)seconds, (unsigned int)(uptime_ms % 1000));
}

uint8_t LOGGER_Init(void) {
    if (is_initialized) {
        // We use LOGGER_Log internally here to print, which relies on is_initialized.
        // So, for this initial message, we use a raw print if UART is ready.
        char msg[] = "[LOGGER] Logger already initialized.\r\n";
        ECUAL_UART_Write(logger_config.uart_id, (uint8_t*)msg, strlen(msg));
        return APP_OK;
    }

    // A basic check to ensure the UART is configured for the logger.
    // In a real system, ECUAL_UART_Init should be called before this.
    // We can't actually check if ECUAL_UART_Write will succeed without a direct ECUAL UART status check.
    // For now, trust that ECUAL_UART_Init has been called.

    is_initialized = true;
    LOGI("LOGGER", "Logger initialized. UART ID: %u, Min Log Level: %s.",
             logger_config.uart_id, get_log_level_str(logger_config.min_serial_log_level));

    return APP_OK;
}

void LOGGER_Log(LOG_LEVEL_T level, const char *tag, const char *format, ...) {
    if (!is_initialized) {
        // Fallback for logging before initialization.
        // This won't have timestamps or full formatting, just raw message to default UART.
        return;
    }

    if (level > logger_config.min_serial_log_level || level == LOG_NONE) {
        return; // Don't log if level is too low or NONE
    }

    va_list args;
    va_start(args, format);

    // Format the message with timestamp, level, and tag
    char timestamp_str[20]; // Example: "[HH:MM:SS.ms]"
    get_timestamp_str(timestamp_str, sizeof(timestamp_str));

    // Calculate space needed for header (timestamp + level + tag + ": ")
    size_t header_len = strlen(timestamp_str) + 1 + strlen(get_log_level_str(level)) + 1 + strlen(tag) + 2;

    int bytes_written_to_buffer = snprintf(log_line_buffer, MAX_LOG_LINE_LENGTH,
                                           "%s %s/%s: ", timestamp_str, get_log_level_str(level), tag);

    if (bytes_written_to_buffer < 0 || bytes_written_to_buffer >= MAX_LOG_LINE_LENGTH) {
        bytes_written_to_buffer = snprintf(log_line_buffer, MAX_LOG_LINE_LENGTH, "[LOG_ERR] %s: ", tag);
        if (bytes_written_to_buffer < 0) bytes_written_to_buffer = 0;
    }

    // Calculate remaining buffer space for the actual message + newline + null terminator
    size_t remaining_space = MAX_LOG_LINE_LENGTH - bytes_written_to_buffer - 1; // -1 for final newline

    // Append the actual formatted message
    int msg_len = vsnprintf(log_line_buffer + bytes_written_to_buffer, remaining_space, format, args);
    va_end(args);

    if (msg_len < 0) {
        const char *error_msg = " (Format error)\r\n";
        size_t error_msg_len = strlen(error_msg);
        if (bytes_written_to_buffer + error_msg_len < MAX_LOG_LINE_LENGTH) {
            memcpy(log_line_buffer + bytes_written_to_buffer, error_msg, error_msg_len);
            bytes_written_to_buffer += error_msg_len;
        }
    } else {
        bytes_written_to_buffer += msg_len;
    }

    // Ensure null termination and add newline if not already present
    if (bytes_written_to_buffer >= MAX_LOG_LINE_LENGTH - 2) {
        bytes_written_to_buffer = MAX_LOG_LINE_LENGTH - 2;
    }

    // Add CRLF (Carriage Return Line Feed) for proper serial terminal display
    log_line_buffer[bytes_written_to_buffer++] = '\r';
    log_line_buffer[bytes_written_to_buffer++] = '\n';
    log_line_buffer[bytes_written_to_buffer] = '\0'; // Null-terminate the string

    // Send the formatted string over UART
    ECUAL_UART_Write(logger_config.uart_id, (uint8_t*)log_line_buffer, bytes_written_to_buffer);
}
