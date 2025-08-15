/**
 * @file hal_wifi_cfg.c
 * @brief Hardware Abstraction Layer for WiFi - Configuration Implementation.
 *
 * This file implements the static configuration parameters for the WiFi module.
 * These settings are used by the `hal_wifi.c` implementation to initialize
 * and operate the WiFi functionalities.
 *
 * IMPORTANT: Adjust these values according to your specific application's
 * WiFi requirements and desired default behavior.
 */

#include "hal_wifi_cfg.h"
#include "hal_wifi.h" // For HAL_WIFI_Mode_t and other enums

// Define the global WiFi configuration structure
const HAL_WIFI_Config_t g_hal_wifi_config = {
    .default_mode = HAL_WIFI_MODE_STA,          // Default to Station mode
    .sta_connect_timeout_ms = 15000,            // 15-second timeout for STA connection
    .sta_max_retries = 5,                       // Max 5 retries for STA connection
    .sta_retry_delay_ms = 2000,                 // 2-second delay between STA retries

    .ap_default_ssid = "ESP32_AP",              // Default SSID for AP mode
    .ap_default_password = "password123",       // Default password for AP mode (min 8 chars for WPA2)
    .ap_default_channel = 6,                    // Default channel for AP mode
    .ap_max_connections = 4,                    // Max 4 stations can connect to the AP
    .ap_hidden_ssid = false,                    // AP SSID is visible by default

    .scan_active_mode = true,                   // Perform active scans by default
    .scan_min_dwell_time_ms = 100,              // Minimum 100ms dwell time per channel during scan
    .scan_max_dwell_time_ms = 300,              // Maximum 300ms dwell time per channel during scan

    // .default_event_callback = NULL,             // No default event callback, application should set it
};

