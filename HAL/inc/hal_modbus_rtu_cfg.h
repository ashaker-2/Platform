/**
 * @file hal_modbus_rtu_cfg.h
 * @brief Configuration header for the HAL Modbus RTU component.
 *
 * This file defines the default and configurable parameters for the
 * HAL Modbus RTU module, such as UART settings, buffer sizes, and timeouts.
 */

#ifndef HAL_MODBUS_RTU_CFG_H
#define HAL_MODBUS_RTU_CFG_H

#include "hal_modbus_rtu.h" // For HAL_ModbusRTU_Role_t and HAL_ModbusRTU_Config_t

// --- Default Modbus RTU Configuration Parameters ---

/**
 * @brief Default baud rate for Modbus RTU communication.
 */
#define HAL_MODBUS_RTU_DEFAULT_BAUD_RATE    9600UL

/**
 * @brief Default number of data bits for Modbus RTU communication.
 */
#define HAL_MODBUS_RTU_DEFAULT_DATA_BITS    8U

/**
 * @brief Default number of stop bits for Modbus RTU communication.
 */
#define HAL_MODBUS_RTU_DEFAULT_STOP_BITS    1U

/**
 * @brief Default parity setting for Modbus RTU communication.
 * 'N' for None, 'E' for Even, 'O' for Odd.
 */
#define HAL_MODBUS_RTU_DEFAULT_PARITY       'N'

/**
 * @brief Default role for the Modbus RTU device upon initialization.
 */
#define HAL_MODBUS_RTU_DEFAULT_ROLE         HAL_MODBUS_RTU_ROLE_SLAVE

/**
 * @brief Default slave address if the device is configured as a Slave.
 * Valid range: 1-247.
 */
#define HAL_MODBUS_RTU_DEFAULT_SLAVE_ADDRESS 1U

/**
 * @brief Default response timeout for Master role in milliseconds.
 * This is the time the Master waits for a response from a Slave.
 */
#define HAL_MODBUS_RTU_DEFAULT_RESPONSE_TIMEOUT_MS 1000UL // 1 second

/**
 * @brief Maximum size of a Modbus RTU frame (including address, function code, data, and 2-byte CRC).
 * Standard Modbus RTU frames are typically up to 256 bytes.
 */
#define HAL_MODBUS_RTU_MAX_FRAME_SIZE       256U

/**
 * @brief Size of the transmit buffer for Modbus RTU frames.
 */
#define HAL_MODBUS_RTU_TX_BUFFER_SIZE       HAL_MODBUS_RTU_MAX_FRAME_SIZE

/**
 * @brief Size of the receive buffer for Modbus RTU frames.
 */
#define HAL_MODBUS_RTU_RX_BUFFER_SIZE       HAL_MODBUS_RTU_MAX_FRAME_SIZE

// --- MCAL UART Configuration (if needed by HAL) ---
// This could be a structure passed to MCAL_UART_Init, or individual defines.
// Example:
// extern const MCAL_UART_Config_t g_mcal_uart_modbus_config;

#endif // HAL_MODBUS_RTU_CFG_H
