/**
 * @file hal_bluetooth_cfg.h
 * @brief Configuration header for the HAL Bluetooth component.
 *
 * This file defines the hardware-specific parameters and default configurations
 * for the HAL Bluetooth module, such as device name, default advertising/scan
 * parameters, and service UUIDs.
 */

#ifndef HAL_BLUETOOTH_CFG_H
#define HAL_BLUETOOTH_CFG_H

#include "hal_bluetooth.h" // For HAL_BLE_Role_t and other HAL BLE types
// #include "bluetooth.h" // For MCAL_BLE_Config_t (MCU-specific BLE driver config)

// --- Default Configuration Parameters ---

/**
 * @brief Default BLE device name to be advertised.
 */
#define HAL_BLE_DEFAULT_DEVICE_NAME         "EnvMon_System"

/**
 * @brief Default BLE role for the system upon initialization.
 */
#define HAL_BLE_DEFAULT_ROLE                HAL_BLE_ROLE_PERIPHERAL

/**
 * @brief Default advertising parameters.
 */
#define HAL_BLE_DEFAULT_ADV_INTERVAL_MIN_MS 1000UL // 1 second
#define HAL_BLE_DEFAULT_ADV_INTERVAL_MAX_MS 1200UL // 1.2 seconds
#define HAL_BLE_DEFAULT_ADV_TYPE            0x00   // ADV_IND (Connectable undirected advertising)
#define HAL_BLE_DEFAULT_ADV_CONNECTABLE     true

/**
 * @brief Default scan parameters.
 */
#define HAL_BLE_DEFAULT_SCAN_INTERVAL_MS    1000UL // 1 second
#define HAL_BLE_DEFAULT_SCAN_WINDOW_MS      500UL  // 0.5 seconds
#define HAL_BLE_DEFAULT_ACTIVE_SCAN         true
#define HAL_BLE_DEFAULT_SCAN_DURATION_S     0      // Continuous scan

/**
 * @brief Default connection parameters.
 */
#define HAL_BLE_DEFAULT_CONN_INTERVAL_MIN_MS 30UL  // 30 ms
#define HAL_BLE_DEFAULT_CONN_INTERVAL_MAX_MS 50UL  // 50 ms
#define HAL_BLE_DEFAULT_CONN_LATENCY        0
#define HAL_BLE_DEFAULT_CONN_TIMEOUT_MS     4000UL // 4 seconds

/**
 * @brief Define a custom service UUID if applicable.
 * Example: 128-bit custom UUID for environmental monitoring service.
 * This should be in little-endian byte order if used directly in advertising data.
 */
#define HAL_BLE_SERVICE_UUID_ENV_MON        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                             0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04} // Example UUID
#define HAL_BLE_SERVICE_UUID_ENV_MON_LEN    16 // Length in bytes

// --- MCAL Bluetooth Configuration (if needed by HAL) ---
// This could be a structure passed to MCAL_BLE_Init, or individual defines.
// Example:
// extern const MCAL_BLE_Config_t g_mcal_ble_config;

#endif // HAL_BLUETOOTH_CFG_H
