/* ============================================================================
 * SOURCE FILE: Application/logger/inc/logger.h
 * ============================================================================*/

#ifndef LOGGER_H
#define LOGGER_H

#include "common.h" // Ensures Status_t is defined
#include <stdint.h>
#include <stdarg.h>

// Include the ESP-IDF logging header directly, as this is a wrapper.
// This allows you to use ESP-IDF's log levels and functionality.
#include "esp_log.h"

// Note: APP_LOG_TAG is typically defined here if you want a *default* tag for all
// logs from this module, but the macros below now allow passing dynamic tags.
// If you want all logs from these macros to *always* use APP_LOG_TAG, then
// remove the 'tag' parameter from the macros and use APP_LOG_TAG instead.
// For now, the 'tag' parameter passed to the macro will be used by ESP_LOGx.
#define APP_LOG_TAG "APP_MAIN" // You can keep this as a fallback/default module tag if desired, though the macros now use the passed 'tag'.

/**
 * @file logger.h
 * @brief Public interface for the Logger component.
 *
 * This header defines the public API for the Logger module, which provides a
 * standardized, flexible logging mechanism.
 */

// --- Log Level Definitions ---
// These are internal log levels for potential custom log handlers or filtering
// if you were to implement LOGGER_Log directly. For macros wrapping ESP_LOGx,
// these are mostly for conceptual alignment.
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,     // Now explicitly included
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,    // Corresponds to ESP_LOGE or special handling
    LOG_LEVEL_NONE,
    LOG_LEVEL_COUNT
} Log_Level_t;

// --- Public Functions ---

/**
 * @brief Initializes the Logger module.
 * This function must be called early in the system startup.
 * @return E_OK on success, E_NOK on failure.
 *
 * NOTE: This function's implementation would typically be in logger.c.
 * It might configure global ESP-IDF log levels, or initialize custom log outputs.
 */
Status_t LOGGER_Init(void);

/**
 * @brief Sets the global log level for the custom logger.
 *
 * If you implement `LOGGER_Log` (currently commented out), this function
 * would control its behavior. For ESP-IDF's built-in logging, you'd typically
 * use `esp_log_level_set(tag, level)` or `LOG_LOCAL_LEVEL` in source files.
 * @param level The new global log level to set.
 */
void LOGGER_SetLogLevel(Log_Level_t level);

// The `LOGGER_Log` function is commented out, implying direct use of ESP_LOG macros.
// If you uncomment and implement this, the macros below would call this function.
/*
void LOGGER_Log(Log_Level_t level, const char* file, int line, const char* format, ...);
*/

// --- Public Logging Macros ---
// These macros now correctly accept 'tag' as their first argument,
// passing it directly to ESP_LOGx. They also automatically prepend
// file/line context to the message.

/**
 * @brief Log an INFO level message.
 * @param tag The ESP-IDF log tag for this message (e.g., "MY_MODULE").
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGI(tag, format, ...) ESP_LOGI(tag, "[%s:%d:%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)


/**
 * @brief Log a WARNING level message.
 * @param tag The ESP-IDF log tag for this message.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGW(tag, format, ...) ESP_LOGW(tag, "[%s:%d:%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief Log an ERROR level message.
 * @param tag The ESP-IDF log tag for this message.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGE(tag, format, ...) ESP_LOGE(tag, "[%s:%d:%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief Log a CRITICAL level message (mapped to ESP_LOGE with a "CRITICAL" prefix).
 * @param tag The ESP-IDF log tag for this message.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGC(tag, format, ...) ESP_LOGE(tag, "[CRITICAL][%s:%d:%s] " format, __FILE__, __LINE__,__func__, ##__VA_ARGS__)

/**
 * @brief Log a DEBUG level message.
 * @param tag The ESP-IDF log tag for this message.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGD(tag, format, ...) ESP_LOGD(tag, "[%s:%d:%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

/**
 * @brief Log a VERBOSE level message.
 * @param tag The ESP-IDF log tag for this message.
 * @param format Printf-like format string.
 * @param ... Variable arguments for the format string.
 */
#define LOGV(tag, format, ...) ESP_LOGV(tag, "[%s:%d:%s] " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif /* LOGGER_H */
