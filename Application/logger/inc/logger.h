#ifndef LOGGER_H
#define LOGGER_H

#include "app_common.h"
#include <stdint.h>
#include <stdarg.h>

/**
 * @file logger.h
 * @brief Public interface for the Logger component.
 *
 * This header defines the public API for the Logger module, which provides a
 * standardized, thread-safe, and flexible logging mechanism.
 */

// --- Log Level Definitions ---
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_NONE,
    LOG_LEVEL_COUNT
} Log_Level_t;

// --- Public Functions ---

/**
 * @brief Initializes the Logger module.
 * This function must be called early in the system startup, before any tasks
 * attempt to log. It initializes the underlying output peripheral (e.g., UART).
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t LOGGER_Init(void);

/**
 * @brief Sets the global log level.
 * Any log messages with a severity lower than this level will be filtered out.
 * @param level The new global log level to set.
 */
void LOGGER_SetLogLevel(Log_Level_t level);

/**
 * @brief Main logging function.
 * This function handles log filtering, formatting, and output. It is not
 * intended to be called directly by application modules. Instead, use the
 * provided LOG macros.
 * @param level The severity level of the log message.
 * @param file The file where the log call originated (usually __FILE__).
 * @param line The line number where the log call originated (usually __LINE__).
 * @param format The format string for the log message.
 * @param ... Variable arguments to be formatted.
 */
void LOGGER_Log(Log_Level_t level, const char* file, int line, const char* format, ...);

// --- Logging Macros ---
#define LOGD(module, format, ...) LOGGER_Log(LOG_LEVEL_DEBUG, module, __LINE__, format, ##__VA_ARGS__)
#define LOGI(module, format, ...) LOGGER_Log(LOG_LEVEL_INFO, module, __LINE__, format, ##__VA_ARGS__)
#define LOGW(module, format, ...) LOGGER_Log(LOG_LEVEL_WARNING, module, __LINE__, format, ##__VA_ARGS__)
#define LOGE(module, format, ...) LOGGER_Log(LOG_LEVEL_ERROR, module, __LINE__, format, ##__VA_ARGS__)
#define LOGF(module, format, ...) LOGGER_Log(LOG_LEVEL_FATAL, module, __LINE__, format, ##__VA_ARGS__)


#endif // LOGGER_H