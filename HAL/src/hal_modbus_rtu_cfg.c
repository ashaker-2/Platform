/**
 * @file hal_modbus_rtu_cfg.c
 * @brief Configuration definitions for the HAL Modbus RTU component.
 *
 * This file contains the actual definitions of the default configuration data
 * for the HAL Modbus RTU module.
 */

#include "hal_modbus_rtu_cfg.h" // Include the configuration header
#include "hal_modbus_rtu.h"     // Include the public header for types

// Define the default Modbus RTU configuration parameters
const HAL_ModbusRTU_Config_t g_hal_modbus_rtu_default_config = {
    .baud_rate          = HAL_MODBUS_RTU_DEFAULT_BAUD_RATE,
    .data_bits          = HAL_MODBUS_RTU_DEFAULT_DATA_BITS,
    .stop_bits          = HAL_MODBUS_RTU_DEFAULT_STOP_BITS,
    .parity             = HAL_MODBUS_RTU_DEFAULT_PARITY,
    .role               = HAL_MODBUS_RTU_DEFAULT_ROLE,
    .slave_address      = HAL_MODBUS_RTU_DEFAULT_SLAVE_ADDRESS,
    .response_timeout_ms = HAL_MODBUS_RTU_DEFAULT_RESPONSE_TIMEOUT_MS
};

// Example for MCAL UART Configuration (if needed)
// const MCAL_UART_Config_t g_mcal_uart_modbus_config = {
//     .uart_instance = MCAL_UART_INSTANCE_MODBUS, // Specific UART instance for Modbus
//     .baud_rate = HAL_MODBUS_RTU_DEFAULT_BAUD_RATE,
//     .data_bits = HAL_MODBUS_RTU_DEFAULT_DATA_BITS,
//     .stop_bits = HAL_MODBUS_RTU_DEFAULT_STOP_BITS,
//     .parity = HAL_MODBUS_RTU_DEFAULT_PARITY,
//     .flow_control = MCAL_UART_FLOW_NONE
//     // ... other MCAL specific settings
// };
