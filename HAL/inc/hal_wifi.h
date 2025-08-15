/**
 * @file hal_wifi.h
 * @brief Hardware Abstraction Layer for WiFi - Interface Header.
 *
 * This header defines the public API for the WiFi HAL, providing an
 * abstract interface for WiFi functionalities such as initialization,
 * connection management, and status retrieval.
 */

#ifndef HAL_WIFI_H
#define HAL_WIFI_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for WiFi HAL status codes.
 */
typedef enum
{
    HAL_WIFI_STATUS_OK = 0,             /**< Operation successful. */
    HAL_WIFI_STATUS_ERROR,              /**< Generic error. */
    HAL_WIFI_STATUS_INVALID_PARAM,      /**< Invalid parameter provided. */
    HAL_WIFI_STATUS_NOT_INITIALIZED,    /**< WiFi HAL not initialized. */
    HAL_WIFI_STATUS_ALREADY_INITIALIZED,/**< WiFi HAL already initialized. */
    HAL_WIFI_STATUS_BUSY,               /**< WiFi operation is busy. */
    HAL_WIFI_STATUS_TIMEOUT,            /**< Operation timed out. */
    HAL_WIFI_STATUS_NOT_CONNECTED,      /**< Not connected to an AP. */
    HAL_WIFI_STATUS_CONNECTED,          /**< Already connected to an AP. */
    HAL_WIFI_STATUS_AP_NOT_FOUND,       /**< Access Point not found during scan/connection. */
    HAL_WIFI_STATUS_WRONG_PASSWORD,     /**< Incorrect password for AP. */
    HAL_WIFI_STATUS_NO_IP,              /**< Failed to obtain an IP address. */
    HAL_WIFI_STATUS_MAX                 /**< Maximum number of status codes. */
} HAL_WIFI_Status_t;

/**
 * @brief Enumeration for WiFi operation modes.
 */
typedef enum
{
    HAL_WIFI_MODE_NONE = 0,     /**< WiFi is disabled. */
    HAL_WIFI_MODE_STA,          /**< Station (client) mode. */
    HAL_WIFI_MODE_AP,           /**< Access Point mode. */
    HAL_WIFI_MODE_APSTA,        /**< Both AP and Station mode. */
    HAL_WIFI_MODE_MAX           /**< Maximum number of WiFi modes. */
} HAL_WIFI_Mode_t;

/**
 * @brief Enumeration for WiFi connection states.
 */
typedef enum
{
    HAL_WIFI_STATE_DISCONNECTED = 0,    /**< WiFi is disconnected. */
    HAL_WIFI_STATE_CONNECTING,          /**< WiFi is in the process of connecting. */
    HAL_WIFI_STATE_CONNECTED,           /**< WiFi is connected and has an IP address. */
    HAL_WIFI_STATE_AP_STARTED,          /**< WiFi AP mode is started. */
    HAL_WIFI_STATE_SCANNING,            /**< WiFi is performing a scan. */
    HAL_WIFI_STATE_MAX                  /**< Maximum number of WiFi states. */
} HAL_WIFI_State_t;

/**
 * @brief Enumeration for WiFi event types.
 */
typedef enum
{
    HAL_WIFI_EVENT_STA_START = 0,       /**< Station mode started. */
    HAL_WIFI_EVENT_STA_STOP,            /**< Station mode stopped. */
    HAL_WIFI_EVENT_STA_CONNECTED,       /**< Station connected to AP. */
    HAL_WIFI_EVENT_STA_DISCONNECTED,    /**< Station disconnected from AP. */
    HAL_WIFI_EVENT_STA_GOT_IP,          /**< Station obtained IP address. */
    HAL_WIFI_EVENT_STA_LOST_IP,         /**< Station lost IP address. */
    HAL_WIFI_EVENT_AP_START,            /**< AP mode started. */
    HAL_WIFI_EVENT_AP_STOP,             /**< AP mode stopped. */
    HAL_WIFI_EVENT_AP_STA_CONNECTED,    /**< A station connected to the AP. */
    HAL_WIFI_EVENT_AP_STA_DISCONNECTED, /**< A station disconnected from the AP. */
    HAL_WIFI_EVENT_SCAN_DONE,           /**< WiFi scan completed. */
    HAL_WIFI_EVENT_MAX                  /**< Maximum number of WiFi events. */
} HAL_WIFI_Event_t;

/**
 * @brief Structure to hold WiFi network information (for scanning).
 */
typedef struct
{
    char ssid[33];          /**< Service Set Identifier (network name). Null-terminated. */
    int8_t rssi;            /**< Received Signal Strength Indicator in dBm. */
    uint8_t channel;        /**< WiFi channel. */
    // Add more fields if needed, e.g., security type
} HAL_WIFI_AP_Info_t;

/**
 * @brief Callback function type for WiFi events.
 *
 * @param event The type of WiFi event that occurred.
 * @param param Optional parameter related to the event (e.g., error code for disconnect).
 */
typedef void (*HAL_WIFI_EventCallback_t)(HAL_WIFI_Event_t event, void *param);

/**
 * @brief Initializes the WiFi HAL.
 *
 * This function performs the necessary setup for the WiFi module,
 * including allocating resources and initializing the network stack.
 * It should be called once before any other WiFi operations.
 *
 * @param mode The desired WiFi operation mode (STA, AP, or APSTA).
 * @param event_callback A callback function to receive WiFi events. Can be NULL.
 * @return HAL_WIFI_STATUS_OK if successful, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_Init(HAL_WIFI_Mode_t mode, HAL_WIFI_EventCallback_t event_callback);

/**
 * @brief De-initializes the WiFi HAL.
 *
 * This function frees resources allocated by the WiFi HAL and stops
 * all WiFi operations.
 *
 * @return HAL_WIFI_STATUS_OK if successful, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_DeInit(void);

/**
 * @brief Connects to a WiFi Access Point (STA mode only).
 *
 * This function attempts to connect the device to a specified WiFi Access Point.
 * It is only applicable when the WiFi is configured in STA or APSTA mode.
 *
 * @param ssid The SSID (network name) of the Access Point.
 * @param password The password for the Access Point (if any). Can be NULL for open networks.
 * @param timeout_ms Connection timeout in milliseconds. Use 0 for no timeout (blocking until connected or failed).
 * @return HAL_WIFI_STATUS_OK if connection initiated successfully, an error code otherwise.
 * Note: For asynchronous connection, the actual connection status will
 * be reported via the event callback (HAL_WIFI_EVENT_STA_CONNECTED/DISCONNECTED/GOT_IP).
 */
HAL_WIFI_Status_t HAL_WIFI_Connect(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * @brief Disconnects from the currently connected WiFi Access Point (STA mode only).
 *
 * This function disconnects the device from the Access Point it is currently
 * connected to.
 *
 * @return HAL_WIFI_STATUS_OK if disconnection initiated successfully, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_Disconnect(void);

/**
 * @brief Starts the WiFi Access Point (AP mode only).
 *
 * This function configures and starts the device as a WiFi Access Point.
 * It is only applicable when the WiFi is configured in AP or APSTA mode.
 *
 * @param ssid The SSID (network name) for the Access Point.
 * @param password The password for the Access Point (min 8 chars, max 64 for WPA2). Can be NULL for open AP.
 * @param channel The WiFi channel for the AP (1-13).
 * @param max_connections Maximum number of stations that can connect to this AP (1-10).
 * @param hidden True if the AP should be hidden, false otherwise.
 * @return HAL_WIFI_STATUS_OK if AP started successfully, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_StartAP(const char *ssid, const char *password, uint8_t channel, uint8_t max_connections, bool hidden);

/**
 * @brief Stops the WiFi Access Point (AP mode only).
 *
 * This function stops the device from operating as a WiFi Access Point.
 *
 * @return HAL_WIFI_STATUS_OK if AP stopped successfully, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_StopAP(void);

/**
 * @brief Performs a WiFi scan for available Access Points.
 *
 * This function initiates a scan for available WiFi networks. The results
 * will be available after the HAL_WIFI_EVENT_SCAN_DONE event is triggered.
 *
 * @param blocking If true, the function blocks until the scan is complete.
 * If false, the scan runs in the background and results are
 * available after the HAL_WIFI_EVENT_SCAN_DONE event.
 * @return HAL_WIFI_STATUS_OK if scan initiated successfully, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_Scan(bool blocking);

/**
 * @brief Retrieves the results of the last WiFi scan.
 *
 * This function populates an array with information about scanned Access Points.
 * It should be called after a scan has completed (e.g., after HAL_WIFI_EVENT_SCAN_DONE).
 *
 * @param ap_info_array Pointer to an array of HAL_WIFI_AP_Info_t structures to fill.
 * @param max_aps The maximum number of APs that can be stored in the array.
 * @param actual_aps Pointer to a variable that will store the actual number of APs found.
 * @return HAL_WIFI_STATUS_OK if successful, an error code otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_GetScanResults(HAL_WIFI_AP_Info_t *ap_info_array, uint16_t max_aps, uint16_t *actual_aps);

/**
 * @brief Gets the current WiFi state.
 *
 * @return The current HAL_WIFI_State_t.
 */
HAL_WIFI_State_t HAL_WIFI_GetState(void);

/**
 * @brief Gets the current IP address of the device in STA mode.
 *
 * @param ip_address_buffer Pointer to a buffer to store the IP address string (e.g., "192.168.1.100").
 * Buffer size should be at least 16 bytes for IPv4.
 * @param buffer_size Size of the ip_address_buffer.
 * @return HAL_WIFI_STATUS_OK if IP address retrieved, HAL_WIFI_STATUS_ERROR if not available or error.
 */
HAL_WIFI_Status_t HAL_WIFI_GetIPAddress(char *ip_address_buffer, uint16_t buffer_size);

/**
 * @brief Gets the current MAC address of the device.
 *
 * @param mac_address_buffer Pointer to a buffer to store the MAC address (6 bytes).
 * @return HAL_WIFI_STATUS_OK if MAC address retrieved, HAL_WIFI_STATUS_ERROR otherwise.
 */
HAL_WIFI_Status_t HAL_WIFI_GetMACAddress(uint8_t *mac_address_buffer);

#endif /* HAL_WIFI_H */
