/**
 * @file hal_bluetooth.c
 * @brief Implementation for the Hardware Abstraction Layer (HAL) Bluetooth component.
 *
 * This file implements the hardware-independent BLE interface, mapping logical
 * BLE operations to their physical MCU counterparts via the MCAL layer. It handles
 * initialization, state management, advertising, scanning, connection management,
 * and data transfer, utilizing callbacks for asynchronous events.
 */

#include "hal_bluetooth.h"      // Public header for HAL_Bluetooth
#include "hal_bluetooth_cfg.h"  // Configuration header for HAL_Bluetooth
// #include "bluetooth.h"     // MCAL layer for direct Bluetooth hardware access
#include "common.h"         // Common application definitions
#include "system_monitor.h"     // For reporting faults
#include "logger.h"             // For logging
#include <string.h>             // For memcpy, memset

// --- Private Data Structures ---
/**
 * @brief Runtime state for the HAL BLE module.
 */
typedef struct {
    HAL_BLE_State_t                 current_state;
    HAL_BLE_Role_t                  current_role;
    bool                            is_initialized;
    bool                            is_connected;
    uint8_t                         peer_address[6]; // Stores connected peer's address

    // Callbacks
    HAL_BLE_StateChangeCallback_t   state_change_cb;
    HAL_BLE_DataReceivedCallback_t  data_received_cb;
    HAL_BLE_ConnectionCallback_t    connection_cb;
    HAL_BLE_ScanResultCallback_t    scan_result_cb;
} HAL_BLE_ModuleState_t;

// --- Private Variables ---
static HAL_BLE_ModuleState_t s_ble_module_state;

// --- Private Function Prototypes ---
static void HAL_BLE_UpdateState(HAL_BLE_State_t new_state);

// MCAL callback handlers (to be registered with MCAL_Bluetooth)
static void MCAL_BLE_StateChangeHandler(HAL_BLE_State_t mcal_state);
static void MCAL_BLE_DataReceivedHandler(const uint8_t *data_p, uint16_t length);
static void MCAL_BLE_ConnectionHandler(bool connected, const uint8_t *peer_address);
static void MCAL_BLE_ScanResultHandler(const uint8_t *peer_address, int8_t rssi,
                                       const uint8_t *adv_data_p, uint16_t adv_data_len);

// --- Public Function Implementations ---

/**
 * @brief Initializes the HAL Bluetooth module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and the underlying MCAL Bluetooth driver.
 * @param role The desired initial BLE role (Peripheral or Central).
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_Init(HAL_BLE_Role_t role) {
    if (s_ble_module_state.is_initialized) {
        // LOG_WARNING("HAL_BLE", "Module already initialized.");
        return E_OK;
    }

    // // Initialize internal state
    // memset(&s_ble_module_state, 0, sizeof(HAL_BLE_ModuleState_t));
    // s_ble_module_state.current_role = role;
    // HAL_BLE_UpdateState(HAL_BLE_STATE_DISCONNECTED); // Initial state

    // // Register HAL's internal handlers with MCAL
    // MCAL_BLE_Callbacks_t mcal_cbs = {
    //     .state_change_cb = MCAL_BLE_StateChangeHandler,
    //     .data_received_cb = MCAL_BLE_DataReceivedHandler,
    //     .connection_cb = MCAL_BLE_ConnectionHandler,
    //     .scan_result_cb = MCAL_BLE_ScanResultHandler
    // };

    // // Initialize MCAL Bluetooth driver
    // MCAL_BLE_Role_t mcal_role = (role == HAL_BLE_ROLE_PERIPHERAL) ? MCAL_BLE_ROLE_PERIPHERAL : MCAL_BLE_ROLE_CENTRAL;
    // if (MCAL_BLE_Init(mcal_role, &mcal_cbs) != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "Failed to initialize MCAL Bluetooth.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 0);
    //     return E_NOK;
    // }

    // s_ble_module_state.is_initialized = true;
    // // LOG_INFO("HAL_BLE", "Module initialized as %s.", (role == HAL_BLE_ROLE_PERIPHERAL) ? "Peripheral" : "Central");
    return E_OK;
}

/**
 * @brief Registers callback functions for BLE events.
 * @param state_change_cb Callback for state changes. Can be NULL.
 * @param data_received_cb Callback for incoming data. Can be NULL.
 * @param connection_cb Callback for connection/disconnection events. Can be NULL.
 * @param scan_result_cb Callback for scan results (only for Central role). Can be NULL.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_RegisterCallbacks(HAL_BLE_StateChangeCallback_t state_change_cb,
                                       HAL_BLE_DataReceivedCallback_t data_received_cb,
                                       HAL_BLE_ConnectionCallback_t connection_cb,
                                       HAL_BLE_ScanResultCallback_t scan_result_cb) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "Cannot register callbacks: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }

    // s_ble_module_state.state_change_cb = state_change_cb;
    // s_ble_module_state.data_received_cb = data_received_cb;
    // s_ble_module_state.connection_cb = connection_cb;
    // s_ble_module_state.scan_result_cb = scan_result_cb;

    // // LOG_DEBUG("HAL_BLE", "Callbacks registered.");
    return E_OK;
}

/**
 * @brief Starts BLE advertising (Peripheral role).
 * @param adv_params Pointer to the advertising parameters.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_StartAdvertising(const HAL_BLE_AdvParams_t *adv_params) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "StartAdvertising failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (s_ble_module_state.current_role != HAL_BLE_ROLE_PERIPHERAL) {
    //     // LOG_ERROR("HAL_BLE", "StartAdvertising failed: Not in Peripheral role.");
    //     return HAL_WRONG_MODE;
    // }
    // if (adv_params == NULL) {
    //     // LOG_ERROR("HAL_BLE", "StartAdvertising failed: adv_params is NULL.");
    //     return E_INVALID_PARAM;
    // }

    // MCAL_BLE_AdvParams_t mcal_adv_params = {
    //     .interval_min_ms = adv_params->interval_min_ms,
    //     .interval_max_ms = adv_params->interval_max_ms,
    //     .type = adv_params->type,
    //     .connectable = adv_params->connectable,
    //     .device_name = adv_params->device_name,
    //     .service_uuid_len = adv_params->service_uuid_len,
    //     .service_uuid = adv_params->service_uuid
    // };

    // if (MCAL_BLE_StartAdvertising(&mcal_adv_params) != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL StartAdvertising failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 1);
    //     return E_NOK;
    // }

    // HAL_BLE_UpdateState(HAL_BLE_STATE_ADVERTISING);
    // // LOG_INFO("HAL_BLE", "Advertising started.");
    return E_OK;
}

/**
 * @brief Stops BLE advertising.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_StopAdvertising(void) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "StopAdvertising failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (s_ble_module_state.current_role != HAL_BLE_ROLE_PERIPHERAL) {
    //     // LOG_WARNING("HAL_BLE", "StopAdvertising called but not in Peripheral role.");
    //     return HAL_WRONG_MODE;
    // }

    // if (MCAL_BLE_StopAdvertising() != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL StopAdvertising failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 2);
    //     return E_NOK;
    // }

    // HAL_BLE_UpdateState(HAL_BLE_STATE_DISCONNECTED); // Or previous state if not advertising
    // // LOG_INFO("HAL_BLE", "Advertising stopped.");
    return E_OK;
}

/**
 * @brief Starts BLE scanning (Central role).
 * @param scan_params Pointer to the scan parameters.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_StartScanning(const HAL_BLE_ScanParams_t *scan_params) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "StartScanning failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (s_ble_module_state.current_role != HAL_BLE_ROLE_CENTRAL) {
    //     // LOG_ERROR("HAL_BLE", "StartScanning failed: Not in Central role.");
    //     return HAL_WRONG_MODE;
    // }
    // if (scan_params == NULL) {
    //     // LOG_ERROR("HAL_BLE", "StartScanning failed: scan_params is NULL.");
    //     return E_INVALID_PARAM;
    // }

    // MCAL_BLE_ScanParams_t mcal_scan_params = {
    //     .interval_ms = scan_params->interval_ms,
    //     .window_ms = scan_params->window_ms,
    //     .active_scan = scan_params->active_scan,
    //     .scan_duration_s = scan_params->scan_duration_s
    // };

    // if (MCAL_BLE_StartScanning(&mcal_scan_params) != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL StartScanning failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 3);
    //     return E_NOK;
    // }

    // HAL_BLE_UpdateState(HAL_BLE_STATE_SCANNING);
    // // LOG_INFO("HAL_BLE", "Scanning started.");
    return E_OK;
}

/**
 * @brief Stops BLE scanning.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_StopScanning(void) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "StopScanning failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (s_ble_module_state.current_role != HAL_BLE_ROLE_CENTRAL) {
    //     // LOG_WARNING("HAL_BLE", "StopScanning called but not in Central role.");
    //     return HAL_WRONG_MODE;
    // }

    // if (MCAL_BLE_StopScanning() != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL StopScanning failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 4);
    //     return E_NOK;
    // }

    // HAL_BLE_UpdateState(HAL_BLE_STATE_DISCONNECTED); // Or previous state if not scanning
    // // LOG_INFO("HAL_BLE", "Scanning stopped.");
    return E_OK;
}

/**
 * @brief Initiates a connection to a peer device (Central role).
 * @param peer_address Pointer to the 6-byte Bluetooth address of the peer.
 * @param conn_params Pointer to the connection parameters.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_Connect(const uint8_t *peer_address, const HAL_BLE_ConnParams_t *conn_params) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "Connect failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (s_ble_module_state.current_role != HAL_BLE_ROLE_CENTRAL) {
    //     // LOG_ERROR("HAL_BLE", "Connect failed: Not in Central role.");
    //     return HAL_WRONG_MODE;
    // }
    // if (peer_address == NULL || conn_params == NULL) {
    //     // LOG_ERROR("HAL_BLE", "Connect failed: peer_address or conn_params is NULL.");
    //     return E_INVALID_PARAM;
    // }

    // MCAL_BLE_ConnParams_t mcal_conn_params = {
    //     .interval_min_ms = conn_params->interval_min_ms,
    //     .interval_max_ms = conn_params->interval_max_ms,
    //     .latency = conn_params->latency,
    //     .timeout_ms = conn_params->timeout_ms
    // };

    // if (MCAL_BLE_Connect(peer_address, &mcal_conn_params) != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL Connect failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 5);
    //     return E_NOK;
    // }

    // HAL_BLE_UpdateState(HAL_BLE_STATE_CONNECTING);
    // memcpy(s_ble_module_state.peer_address, peer_address, 6);
    // // LOG_INFO("HAL_BLE", "Attempting to connect to peer.");
    return E_OK;
}

/**
 * @brief Disconnects from the currently connected peer.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_Disconnect(void) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "Disconnect failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (!s_ble_module_state.is_connected) {
    //     // LOG_WARNING("HAL_BLE", "Disconnect called but not connected.");
    //     return E_OK; // Already disconnected, consider it success
    // }

    // if (MCAL_BLE_Disconnect() != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL Disconnect failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 6);
    //     return E_NOK;
    // }

    // // State update will happen via MCAL_BLE_ConnectionHandler callback
    // // LOG_INFO("HAL_BLE", "Disconnecting from peer.");
    return E_OK;
}

/**
 * @brief Sends data over the established BLE connection.
 * @param data_p Pointer to the data buffer to send.
 * @param length The number of bytes to send.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_BLE_SendData(const uint8_t *data_p, uint16_t length) 
{
    // if (!s_ble_module_state.is_initialized) {
    //     // LOG_ERROR("HAL_BLE", "SendData failed: Module not initialized.");
    //     return HAL_NOT_INITIALIZED;
    // }
    // if (!s_ble_module_state.is_connected) {
    //     // LOG_ERROR("HAL_BLE", "SendData failed: Not connected to a peer.");
    //     return HAL_NOT_CONNECTED;
    // }
    // if (data_p == NULL || length == 0) {
    //     // LOG_ERROR("HAL_BLE", "SendData failed: NULL data_p or zero length.");
    //     return E_INVALID_PARAM;
    // }

    // if (MCAL_BLE_SendData(data_p, length) != E_OK) {
    //     // LOG_ERROR("HAL_BLE", "MCAL SendData failed.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_BLE_ERROR, 7);
    //     return E_NOK;
    // }
    // // LOG_VERBOSE("HAL_BLE", "Sent %u bytes over BLE.", length);
    return E_OK;
}

/**
 * @brief Gets the current BLE state.
 * @return The current HAL_BLE_State_t.
 */
HAL_BLE_State_t HAL_BLE_GetState(void) {
    return s_ble_module_state.current_state;
}

// --- Private Function Implementations ---

/**
 * @brief Updates the internal BLE state and notifies the registered callback.
 * @param new_state The new BLE state to set.
 */
static void HAL_BLE_UpdateState(HAL_BLE_State_t new_state)
{
    // if (s_ble_module_state.current_state != new_state) {
    //     // LOG_DEBUG("HAL_BLE", "State changed from %d to %d.", s_ble_module_state.current_state, new_state);
    //     s_ble_module_state.current_state = new_state;
    //     if (s_ble_module_state.state_change_cb != NULL) {
    //         s_ble_module_state.state_change_cb(new_state);
    //     }
    // }
}

// --- MCAL Callback Handlers (Internal to HAL) ---

/**
 * @brief Handler for state changes reported by the MCAL Bluetooth driver.
 * Translates MCAL states to HAL states and updates the module.
 * @param mcal_state The state reported by the MCAL.
 */
static void MCAL_BLE_StateChangeHandler(HAL_BLE_State_t mcal_state) 
{
    // HAL_BLE_State_t hal_state;
    // switch (mcal_state) {
    //     case MCAL_BLE_STATE_IDLE:
    //         hal_state = HAL_BLE_STATE_DISCONNECTED;
    //         break;
    //     case MCAL_BLE_STATE_ADVERTISING:
    //         hal_state = HAL_BLE_STATE_ADVERTISING;
    //         break;
    //     case MCAL_BLE_STATE_SCANNING:
    //         hal_state = HAL_BLE_STATE_SCANNING;
    //         break;
    //     case MCAL_BLE_STATE_CONNECTING:
    //         hal_state = HAL_BLE_STATE_CONNECTING;
    //         break;
    //     case MCAL_BLE_STATE_CONNECTED:
    //         hal_state = HAL_BLE_STATE_CONNECTED;
    //         break;
    //     case MCAL_BLE_STATE_PAIRING:
    //         hal_state = HAL_BLE_STATE_PAIRING;
    //         break;
    //     case MCAL_BLE_STATE_ERROR:
    //     default:
    //         hal_state = HAL_BLE_STATE_ERROR;
    //         break;
    // }
    // HAL_BLE_UpdateState(hal_state);
}

/**
 * @brief Handler for incoming data reported by the MCAL Bluetooth driver.
 * @param data_p Pointer to the received data.
 * @param length Length of the received data.
 */
static void MCAL_BLE_DataReceivedHandler(const uint8_t *data_p, uint16_t length) 
{
    // if (s_ble_module_state.data_received_cb != NULL) {
    //     s_ble_module_state.data_received_cb(data_p, length);
    // } else {
    //     // LOG_DEBUG("HAL_BLE", "Data received (%u bytes) but no handler registered.", length);
    // }
}

/**
 * @brief Handler for connection/disconnection events reported by the MCAL Bluetooth driver.
 * @param connected True if connected, false if disconnected.
 * @param peer_address Pointer to the peer's Bluetooth address.
 */
static void MCAL_BLE_ConnectionHandler(bool connected, const uint8_t *peer_address) 
{
    // s_ble_module_state.is_connected = connected;
    // if (connected) {
    //     memcpy(s_ble_module_state.peer_address, peer_address, 6);
    //     HAL_BLE_UpdateState(HAL_BLE_STATE_CONNECTED);
    //     // LOG_INFO("HAL_BLE", "Connected to peer: %02X:%02X:%02X:%02X:%02X:%02X",
    //              peer_address[5], peer_address[4], peer_address[3],
    //              peer_address[2], peer_address[1], peer_address[0]);
    // } else {
    //     memset(s_ble_module_state.peer_address, 0, 6); // Clear peer address on disconnect
    //     HAL_BLE_UpdateState(HAL_BLE_STATE_DISCONNECTED);
    //     // LOG_INFO("HAL_BLE", "Disconnected from peer.");
    // }

    // if (s_ble_module_state.connection_cb != NULL) {
    //     s_ble_module_state.connection_cb(connected, peer_address);
    // }
}

/**
 * @brief Handler for scan results reported by the MCAL Bluetooth driver.
 * @param peer_address Pointer to the discovered device's Bluetooth address.
 * @param rssi Received Signal Strength Indicator.
 * @param adv_data_p Pointer to the raw advertising data.
 * @param adv_data_len Length of the advertising data.
 */
static void MCAL_BLE_ScanResultHandler(const uint8_t *peer_address, int8_t rssi,
                                       const uint8_t *adv_data_p, uint16_t adv_data_len) 
{
    // if (s_ble_module_state.scan_result_cb != NULL) {
    //     s_ble_module_state.scan_result_cb(peer_address, rssi, adv_data_p, adv_data_len);
    // } else {
    //     // LOG_DEBUG("HAL_BLE", "Scan result for %02X:%02X:%02X:%02X:%02X:%02X (RSSI: %d) but no handler registered.",
    //               peer_address[5], peer_address[4], peer_address[3],
    //               peer_address[2], peer_address[1], peer_address[0], rssi);
    // }
}