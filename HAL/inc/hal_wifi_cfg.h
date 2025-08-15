/**
 * @file hal_wifi_cfg.h
 * @brief Hardware Abstraction Layer for WiFi - Configuration Header.
 *
 * This header defines the compile-time configuration parameters for the
 * WiFi module. These settings are typically defined in `hal_wifi_cfg.c`
 * and are used during the initialization of the WiFi HAL.
 */

#ifndef HAL_WIFI_CFG_H
#define HAL_WIFI_CFG_H

#include <stdint.h>
#include <stdbool.h>
#include "hal_wifi.h" // Include the main WiFi HAL interface for enums and types

/**
 * @brief Structure to hold compile-time configuration for the WiFi module.
 *
 * This structure defines various parameters that configure the WiFi behavior,
 * such as default mode, AP settings, and connection retry limits.
 */
typedef struct
{
    HAL_WIFI_Mode_t default_mode;           /**< Default WiFi mode on initialization (STA, AP, APSTA). */
    uint32_t sta_connect_timeout_ms;        /**< Default timeout for STA connection attempts in milliseconds. */
    uint8_t sta_max_retries;                /**< Maximum number of retries for STA connection. */
    uint32_t sta_retry_delay_ms;            /**< Delay between STA connection retries in milliseconds. */

    // AP Mode Configuration
    const char *ap_default_ssid;            /**< Default SSID for AP mode. */
    const char *ap_default_password;        /**< Default password for AP mode (NULL for open). */
    uint8_t ap_default_channel;             /**< Default channel for AP mode (1-13). */
    uint8_t ap_max_connections;             /**< Maximum number of stations that can connect to the AP (1-10). */
    bool ap_hidden_ssid;                    /**< True if the AP SSID should be hidden by default. */

    // Scan Configuration
    bool scan_active_mode;                  /**< True for active scan, false for passive scan. */
    uint32_t scan_min_dwell_time_ms;        /**< Minimum dwell time per channel during scan (ms). */
    uint32_t scan_max_dwell_time_ms;        /**< Maximum dwell time per channel during scan (ms). */

    // Event Callback (can be overridden during HAL_WIFI_Init)
    HAL_WIFI_EventCallback_t default_event_callback; /**< Default callback for WiFi events. Can be NULL. */

} HAL_WIFI_Config_t;

/**
 * @brief External declaration for the global WiFi configuration structure.
 *
 * This structure is defined in `hal_wifi_cfg.c` and provides the static
 * configuration for the WiFi module.
 */
extern const HAL_WIFI_Config_t g_hal_wifi_config;

#endif /* HAL_WIFI_CFG_H */
