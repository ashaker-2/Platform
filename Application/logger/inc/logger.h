// app/inc/logger.h

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "app_common.h" // For APP_OK/APP_ERROR
#include "ecual_uart.h" // For ECUAL_UART_ID_t

/**
 * @brief Custom log levels for the generic logger.
 */
typedef enum {
    LOG_NONE = 0,    ///< No log output
    LOG_ERROR,       ///< Error messages
    LOG_WARN,        ///< Warning messages
    LOG_INFO,        ///< Informational messages
    LOG_DEBUG,       ///< Debugging messages
    LOG_VERBOSE,     ///< Verbose messages
} LOG_LEVEL_T;

/**
 * @brief Configuration structure for the generic Logger component.
 */
typedef struct {
    ECUAL_UART_ID_t   uart_id;            ///< The ECUAL UART bus ID to use for logging.
    LOG_LEVEL_T   min_serial_log_level; ///< Minimum log level to be printed to serial.
} LOGGER_Config_t;

/**
 * @brief Initializes the Logger component.
 * This sets up the serial logging mechanism.
 * Ensure the specified ECUAL_UART_ID_t is initialized before calling this.
 * @return APP_OK if initialized successfully, APP_ERROR otherwise.
 */
uint8_t LOGGER_Init(void);

/**
 * @brief Logs a message with a specified level and tag.
 * The message will be printed to the configured serial port if its level
 * meets or exceeds the minimum serial log level.
 * @param level The log level (e.g., LOG_INFO, LOG_ERROR).
 * @param tag The tag for the log message (e.g., "MAIN", "HEATER").
 * @param format Format string for the message.
 * @param ... Variable arguments for the format string.
 */
void LOGGER_Log(LOG_LEVEL_T level, const char *tag, const char *format, ...);

// Macros for convenience, replacing esp_logx
#define LOGE(tag, format, ...) LOGGER_Log(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) LOGGER_Log(LOG_WARN, tag, format, ##__VA_ARGS__)
#define LOGI(tag, format, ...) LOGGER_Log(LOG_INFO, tag, format, ##__VA_ARGS__)
#define LOGD(tag, format, ...) LOGGER_Log(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define LOGV(tag, format, ...) LOGGER_Log(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#endif /* LOGGER_H */
