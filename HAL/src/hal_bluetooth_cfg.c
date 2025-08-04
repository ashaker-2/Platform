/**
 * @file hal_bluetooth_cfg.c
 * @brief Configuration definitions for the HAL Bluetooth component.
 *
 * This file contains the actual definitions of the configuration data
 * for the HAL Bluetooth module, such as default advertising and scan parameters.
 */

#include "hal_bluetooth_cfg.h" // Include the configuration header
#include "hal_bluetooth.h"     // Include the public header for types

// Define the custom service UUID data
static const uint8_t s_env_mon_service_uuid[] = HAL_BLE_SERVICE_UUID_ENV_MON;

// Define default advertising parameters
const HAL_BLE_AdvParams_t g_hal_ble_default_adv_params = {
    .interval_min_ms    = HAL_BLE_DEFAULT_ADV_INTERVAL_MIN_MS,
    .interval_max_ms    = HAL_BLE_DEFAULT_ADV_INTERVAL_MAX_MS,
    .type               = HAL_BLE_DEFAULT_ADV_TYPE,
    .connectable        = HAL_BLE_DEFAULT_ADV_CONNECTABLE,
    .device_name        = HAL_BLE_DEFAULT_DEVICE_NAME,
    .service_uuid_len   = HAL_BLE_SERVICE_UUID_ENV_MON_LEN,
    .service_uuid       = s_env_mon_service_uuid
};

// Define default scan parameters
const HAL_BLE_ScanParams_t g_hal_ble_default_scan_params = {
    .interval_ms        = HAL_BLE_DEFAULT_SCAN_INTERVAL_MS,
    .window_ms          = HAL_BLE_DEFAULT_SCAN_WINDOW_MS,
    .active_scan        = HAL_BLE_DEFAULT_ACTIVE_SCAN,
    .scan_duration_s    = HAL_BLE_DEFAULT_SCAN_DURATION_S
};

// Define default connection parameters
const HAL_BLE_ConnParams_t g_hal_ble_default_conn_params = {
    .interval_min_ms    = HAL_BLE_DEFAULT_CONN_INTERVAL_MIN_MS,
    .interval_max_ms    = HAL_BLE_DEFAULT_CONN_INTERVAL_MAX_MS,
    .latency            = HAL_BLE_DEFAULT_CONN_LATENCY,
    .timeout_ms         = HAL_BLE_DEFAULT_CONN_TIMEOUT_MS
};

// Example for MCAL Bluetooth Configuration (if needed)
// const MCAL_BLE_Config_t g_mcal_ble_config = {
//     .baud_rate = 115200,
//     .flow_control_enabled = true,
//     // ... other MCAL specific settings
// };
