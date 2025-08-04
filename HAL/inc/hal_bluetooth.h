/**
 * @file hal_bluetooth.h
 * @brief Public header for the Hardware Abstraction Layer (HAL) Bluetooth component.
 *
 * This component provides a hardware-independent interface for interacting with
 * a Bluetooth Low Energy (BLE) module. It abstracts the specific commands and
 * protocols of the underlying BLE chip and provides a clean API for higher-level
 * modules to manage BLE connections, send/receive data, and handle advertisements.
 */

#ifndef HAL_BLUETOOTH_H
#define HAL_BLUETOOTH_H

#include "app_common.h" // For APP_Status_t and HAL_Status_t
#include <stdint.h>     // For uint8_t, uint16_t, uint32_t
#include <stdbool.h>    // For bool

// --- Type Definitions ---

/**
 * @brief Enumeration for BLE connection states.
 */
typedef enum
{
    HAL_BLE_STATE_DISCONNECTED = 0, /**< BLE is not connected to any peer. */
    HAL_BLE_STATE_ADVERTISING,      /**< BLE is currently advertising. */
    HAL_BLE_STATE_SCANNING,         /**< BLE is currently scanning for devices. */
    HAL_BLE_STATE_CONNECTING,       /**< BLE is attempting to connect to a peer. */
    HAL_BLE_STATE_CONNECTED,        /**< BLE is connected to a peer device. */
    HAL_BLE_STATE_PAIRING,          /**< BLE is in the process of pairing. */
    HAL_BLE_STATE_ERROR             /**< An error state has occurred. */
} HAL_BLE_State_t;

/**
 * @brief Enumeration for BLE roles.
 */
typedef enum
{
    HAL_BLE_ROLE_PERIPHERAL = 0, /**< Device acts as a peripheral (advertises, accepts connections). */
    HAL_BLE_ROLE_CENTRAL,        /**< Device acts as a central (scans, initiates connections). */
    HAL_BLE_ROLE_COUNT
} HAL_BLE_Role_t;

/**
 * @brief Structure to hold BLE advertising parameters.
 */
typedef struct
{
    uint16_t interval_min_ms;    /**< Minimum advertising interval in milliseconds. */
    uint16_t interval_max_ms;    /**< Maximum advertising interval in milliseconds. */
    uint8_t type;                /**< Advertising type (e.g., connectable, scannable, non-connectable). */
    bool connectable;            /**< True if the device can be connected to. */
    const char *device_name;     /**< Device name to advertise. */
    uint8_t service_uuid_len;    /**< Length of the service UUID (0 if none). */
    const uint8_t *service_uuid; /**< Pointer to the service UUID data. */
} HAL_BLE_AdvParams_t;

/**
 * @brief Structure to hold BLE scan parameters.
 */
typedef struct
{
    uint16_t interval_ms;     /**< Scan interval in milliseconds. */
    uint16_t window_ms;       /**< Scan window in milliseconds. */
    bool active_scan;         /**< True for active scanning (sends scan requests). */
    uint16_t scan_duration_s; /**< Duration of the scan in seconds (0 for continuous). */
} HAL_BLE_ScanParams_t;

/**
 * @brief Structure to hold BLE connection parameters.
 */
typedef struct
{
    uint16_t interval_min_ms; /**< Minimum connection interval in milliseconds. */
    uint16_t interval_max_ms; /**< Maximum connection interval in milliseconds. */
    uint16_t latency;         /**< Slave latency. */
    uint16_t timeout_ms;      /**< Connection supervision timeout in milliseconds. */
} HAL_BLE_ConnParams_t;

// --- Callback Function Pointers ---

/**
 * @brief Callback function type for BLE state changes.
 * @param new_state The new BLE state.
 */
typedef void (*HAL_BLE_StateChangeCallback_t)(HAL_BLE_State_t new_state);

/**
 * @brief Callback function type for incoming BLE data.
 * @param data_p Pointer to the received data buffer.
 * @param length The length of the received data.
 */
typedef void (*HAL_BLE_DataReceivedCallback_t)(const uint8_t *data_p, uint16_t length);

/**
 * @brief Callback function type for BLE connection events.
 * @param connected True if connected, false if disconnected.
 * @param peer_address Pointer to the peer's Bluetooth address (6 bytes).
 */
typedef void (*HAL_BLE_ConnectionCallback_t)(bool connected, const uint8_t *peer_address);

/**
 * @brief Callback function type for BLE scan results.
 * @param peer_address Pointer to the discovered device's Bluetooth address (6 bytes).
 * @param rssi Received Signal Strength Indicator.
 * @param adv_data_p Pointer to the raw advertising data.
 * @param adv_data_len Length of the advertising data.
 */
typedef void (*HAL_BLE_ScanResultCallback_t)(const uint8_t *peer_address, int8_t rssi,
                                             const uint8_t *adv_data_p, uint16_t adv_data_len);

// --- Public Functions ---

/**
 * @brief Initializes the HAL Bluetooth module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and the underlying MCAL Bluetooth driver.
 * @param role The desired initial BLE role (Peripheral or Central).
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_Init(HAL_BLE_Role_t role);

/**
 * @brief Registers callback functions for BLE events.
 * @param state_change_cb Callback for state changes. Can be NULL.
 * @param data_received_cb Callback for incoming data. Can be NULL.
 * @param connection_cb Callback for connection/disconnection events. Can be NULL.
 * @param scan_result_cb Callback for scan results (only for Central role). Can be NULL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_RegisterCallbacks(HAL_BLE_StateChangeCallback_t state_change_cb,
                                       HAL_BLE_DataReceivedCallback_t data_received_cb,
                                       HAL_BLE_ConnectionCallback_t connection_cb,
                                       HAL_BLE_ScanResultCallback_t scan_result_cb);

/**
 * @brief Starts BLE advertising (Peripheral role).
 * @param adv_params Pointer to the advertising parameters.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_StartAdvertising(const HAL_BLE_AdvParams_t *adv_params);

/**
 * @brief Stops BLE advertising.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_StopAdvertising(void);

/**
 * @brief Starts BLE scanning (Central role).
 * @param scan_params Pointer to the scan parameters.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_StartScanning(const HAL_BLE_ScanParams_t *scan_params);

/**
 * @brief Stops BLE scanning.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_StopScanning(void);

/**
 * @brief Initiates a connection to a peer device (Central role).
 * @param peer_address Pointer to the 6-byte Bluetooth address of the peer.
 * @param conn_params Pointer to the connection parameters.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_Connect(const uint8_t *peer_address, const HAL_BLE_ConnParams_t *conn_params);

/**
 * @brief Disconnects from the currently connected peer.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_Disconnect(void);

/**
 * @brief Sends data over the established BLE connection.
 * @param data_p Pointer to the data buffer to send.
 * @param length The number of bytes to send.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_BLE_SendData(const uint8_t *data_p, uint16_t length);

/**
 * @brief Gets the current BLE state.
 * @return The current HAL_BLE_State_t.
 */
HAL_BLE_State_t HAL_BLE_GetState(void);

#endif // HAL_BLUETOOTH_H
