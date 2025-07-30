// ecual/inc/ecual_uart_config.h

#ifndef ECUAL_UART_CONFIG_H
#define ECUAL_UART_CONFIG_H

#include <stdint.h>
#include "ecual_uart.h" // Include ecual_uart.h for ECUAL_UART_Config_t definition

/**
 * @brief Define Symbolic Names for your UART instances.
 * These map to ECUAL_UART_UNIT_0, ECUAL_UART_UNIT_1, ECUAL_UART_UNIT_2.
 */
#define ECUAL_UART_DEBUG_CONSOLE    ECUAL_UART_UNIT_0
#define ECUAL_UART_GPS_MODULE       ECUAL_UART_UNIT_1
#define ECUAL_UART_BLUETOOTH_MODULE ECUAL_UART_UNIT_2


// Declare the external constant array for UART configurations
extern const ECUAL_UART_Config_t uart_channel_configurations[];
// Declare the external constant for its size
extern const uint32_t ECUAL_NUM_UART_CONFIGURATIONS;

#endif /* ECUAL_UART_CONFIG_H */
