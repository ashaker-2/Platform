// app/src/logger_config.c

#include "logger_config.h"

const LOGGER_Config_t logger_config = {
    .uart_id            = ECUAL_UART_0,      // Use UART_0 for logging (or your desired UART)
    .min_serial_log_level = LOG_INFO     // Only log INFO, WARN, ERROR, DEBUG, VERBOSE messages to serial
};
