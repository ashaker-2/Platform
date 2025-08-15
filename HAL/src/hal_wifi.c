/**
 * @file hal_wifi.c
 * @brief Hardware Abstraction Layer for WiFi - Implementation.
 *
 * This module provides the concrete implementation for the WiFi HAL interface
 * defined in `hal_wifi.h`. It utilizes the ESP-IDF WiFi driver and Event Loop
 * to manage WiFi functionalities on the ESP32.
 */

#include "hal_wifi.h"
#include "hal_wifi_cfg.h" // Include configuration header

#include <string.h>
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "esp_netif.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

// Define a tag for ESP-IDF logging
static const char *TAG = "HAL_WIFI";

// Event Group for WiFi events
// static EventGroupHandle_t s_wifi_event_group;

// Bits for the event group
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_AP_STARTED_BIT BIT2
#define WIFI_SCAN_DONE_BIT BIT3

// Static variables to hold WiFi state and callback
// static HAL_WIFI_State_t s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
// static HAL_WIFI_EventCallback_t s_event_callback = NULL;
// static bool s_wifi_initialized = false;
// static esp_netif_t *s_esp_netif_sta = NULL;
// static esp_netif_t *s_esp_netif_ap = NULL;

// Static variables for STA connection retries
static int s_retry_num = 0;

// Static buffer for scan results
#define MAX_SCAN_APS 20
static HAL_WIFI_AP_Info_t s_scan_results[MAX_SCAN_APS];
static uint16_t s_actual_scan_aps = 0;

/**
 * @brief Internal event handler for WiFi and IP events.
 *
 * This function handles various WiFi and IP-related events from the ESP-IDF
 * event loop and translates them into HAL_WIFI_Event_t for the application
 * callback.
 *
 * @param arg User context (not used here).
 * @param event_base The event base (e.g., WIFI_EVENT, IP_EVENT).
 * @param event_id The specific event ID.
 * @param event_data Data associated with the event.
 */
static void wifi_event_handler(void *arg, int32_t event_base,int32_t event_id, void *event_data)
{
    // if (event_base == WIFI_EVENT)
    // {
    //     switch (event_id)
    //     {
    //     case WIFI_EVENT_STA_START:
    //         LOGI(TAG, "WIFI_EVENT_STA_START");
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_START, NULL);
    //         break;
    //     case WIFI_EVENT_STA_STOP:
    //         LOGI(TAG, "WIFI_EVENT_STA_STOP");
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_STOP, NULL);
    //         break;
    //     case WIFI_EVENT_STA_CONNECTED:
    //         LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
    //         s_current_wifi_state = HAL_WIFI_STATE_CONNECTED; // Connected to AP, waiting for IP
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_CONNECTED, NULL);
    //         break;
    //     case WIFI_EVENT_STA_DISCONNECTED:
    //     {
    //         wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;
    //         LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED, reason: %d", disconnected->reason);
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //         xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

    //         if (s_retry_num < g_hal_wifi_config.sta_max_retries)
    //         {
    //             esp_wifi_connect();
    //             s_retry_num++;
    //             LOGI(TAG, "Retrying to connect to the AP (%d/%d)...", s_retry_num, g_hal_wifi_config.sta_max_retries);
    //         }
    //         else
    //         {
    //             xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    //             ESP_LOGE(TAG, "Failed to connect to AP after %d retries.", g_hal_wifi_config.sta_max_retries);
    //         }
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_DISCONNECTED, (void *)&disconnected->reason);
    //         break;
    //     }
    //     case WIFI_EVENT_AP_START:
    //         LOGI(TAG, "WIFI_EVENT_AP_START");
    //         s_current_wifi_state = HAL_WIFI_STATE_AP_STARTED;
    //         xEventGroupSetBits(s_wifi_event_group, WIFI_AP_STARTED_BIT);
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_AP_START, NULL);
    //         break;
    //     case WIFI_EVENT_AP_STOP:
    //         LOGI(TAG, "WIFI_EVENT_AP_STOP");
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_AP_STOP, NULL);
    //         break;
    //     case WIFI_EVENT_AP_STACONNECTED:
    //     {
    //         wifi_event_ap_staconnected_t *connected = (wifi_event_ap_staconnected_t *)event_data;
    //         LOGI(TAG, "station " MACSTR " join, AID=%d",
    //              MAC2STR(connected->mac), connected->aid);
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_AP_STA_CONNECTED, NULL);
    //         break;
    //     }
    //     case WIFI_EVENT_AP_STADISCONNECTED:
    //     {
    //         wifi_event_ap_stadisconnected_t *disconnected = (wifi_event_ap_stadisconnected_t *)event_data;
    //         LOGI(TAG, "station " MACSTR " leave, AID=%d",
    //              MAC2STR(disconnected->mac), disconnected->aid);
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_AP_STA_DISCONNECTED, NULL);
    //         break;
    //     }
    //     case WIFI_EVENT_SCAN_DONE:
    //     {
    //         LOGI(TAG, "WIFI_EVENT_SCAN_DONE");
    //         uint16_t ap_count = 0;
    //         esp_wifi_scan_get_ap_num(&ap_count);
    //         if (ap_count == 0)
    //         {
    //             LOGI(TAG, "No APs found.");
    //             s_actual_scan_aps = 0;
    //         }
    //         else
    //         {
    //             wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    //             if (ap_records)
    //             {
    //                 esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    //                 s_actual_scan_aps = 0;
    //                 for (int i = 0; i < ap_count && s_actual_scan_aps < MAX_SCAN_APS; i++)
    //                 {
    //                     memcpy(s_scan_results[s_actual_scan_aps].ssid, (char *)ap_records[i].ssid, sizeof(s_scan_results[s_actual_scan_aps].ssid));
    //                     s_scan_results[s_actual_scan_aps].ssid[sizeof(s_scan_results[s_actual_scan_aps].ssid) - 1] = '\0'; // Ensure null-termination
    //                     s_scan_results[s_actual_scan_aps].rssi = ap_records[i].rssi;
    //                     s_scan_results[s_actual_scan_aps].channel = ap_records[i].primary;
    //                     s_actual_scan_aps++;
    //                     ESP_LOGD(TAG, "AP found: SSID %s, RSSI %d, Channel %d",
    //                              s_scan_results[s_actual_scan_aps - 1].ssid,
    //                              s_scan_results[s_actual_scan_aps - 1].rssi,
    //                              s_scan_results[s_actual_scan_aps - 1].channel);
    //                 }
    //                 free(ap_records);
    //             }
    //             else
    //             {
    //                 ESP_LOGE(TAG, "Failed to allocate memory for AP records.");
    //                 s_actual_scan_aps = 0;
    //             }
    //         }
    //         xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED; // Reset state after scan
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_SCAN_DONE, NULL);
    //         break;
    //     }
    //     default:
    //         break;
    //     }
    // }
    // else if (event_base == IP_EVENT)
    // {
    //     switch (event_id)
    //     {
    //     case IP_EVENT_STA_GOT_IP:
    //         LOGI(TAG, "IP_EVENT_STA_GOT_IP");
    //         ip_event_got_ip_t *ip_event = (ip_event_got_ip_t *)event_data;
    //         LOGI(TAG, "Got IP: " IPSTR, IP2STR(&ip_event->ip_info.ip));
    //         s_retry_num = 0;
    //         xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    //         s_current_wifi_state = HAL_WIFI_STATE_CONNECTED;
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_GOT_IP, NULL);
    //         break;
    //     case IP_EVENT_STA_LOST_IP:
    //         LOGI(TAG, "IP_EVENT_STA_LOST_IP");
    //         s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //         xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    //         if (s_event_callback)
    //             s_event_callback(HAL_WIFI_EVENT_STA_LOST_IP, NULL);
    //         break;
    //     default:
    //         break;
    //     }
    // }
}

HAL_WIFI_Status_t HAL_WIFI_Init(HAL_WIFI_Mode_t mode, HAL_WIFI_EventCallback_t event_callback)
{
    // if (s_wifi_initialized)
    // {
    //     ESP_LOGW(TAG, "WiFi HAL already initialized.");
    //     return HAL_WIFI_STATUS_ALREADY_INITIALIZED;
    // }

    // if (mode >= HAL_WIFI_MODE_MAX || mode == HAL_WIFI_MODE_NONE)
    // {
    //     ESP_LOGE(TAG, "Invalid WiFi mode specified for initialization: %d", mode);
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // s_event_callback = (event_callback != NULL) ? event_callback : g_hal_wifi_config.default_event_callback;

    // // Create the FreeRTOS event group
    // s_wifi_event_group = xEventGroupCreate();
    // if (s_wifi_event_group == NULL)
    // {
    //     ESP_LOGE(TAG, "Failed to create WiFi event group.");
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Initialize the network interface (TCP/IP stack)
    // esp_err_t ret = esp_netif_init();
    // if (ret != ESP_OK && ret != ESP_ERR_NVS_NO_FREE_PAGES && ret != ESP_ERR_NVS_NEW_VERSION_FOUND) // Allow existing init or NVS issues
    // {
    //     ESP_LOGE(TAG, "Failed to initialize network interface: %s", esp_err_to_name(ret));
    //     vEventGroupDelete(s_wifi_event_group);
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Create the default event loop
    // ret = esp_event_loop_create_default();
    // if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) // Allow existing loop
    // {
    //     ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(ret));
    //     esp_netif_deinit();
    //     vEventGroupDelete(s_wifi_event_group);
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Register event handlers
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // ret = esp_wifi_init(&cfg);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to initialize ESP WiFi: %s", esp_err_to_name(ret));
    //     esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);
    //     esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler);
    //     esp_event_loop_delete_default();
    //     esp_netif_deinit();
    //     vEventGroupDelete(s_wifi_event_group);
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Set WiFi mode and create netif instances
    // switch (mode)
    // {
    // case HAL_WIFI_MODE_STA:
    //     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //     s_esp_netif_sta = esp_netif_create_default_wifi_sta();
    //     if (s_esp_netif_sta == NULL)
    //     {
    //         ESP_LOGE(TAG, "Failed to create STA netif.");
    //         HAL_WIFI_DeInit(); // Clean up
    //         return HAL_WIFI_STATUS_ERROR;
    //     }
    //     break;
    // case HAL_WIFI_MODE_AP:
    //     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    //     s_esp_netif_ap = esp_netif_create_default_wifi_ap();
    //     if (s_esp_netif_ap == NULL)
    //     {
    //         ESP_LOGE(TAG, "Failed to create AP netif.");
    //         HAL_WIFI_DeInit(); // Clean up
    //         return HAL_WIFI_STATUS_ERROR;
    //     }
    //     break;
    // case HAL_WIFI_MODE_APSTA:
    //     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    //     s_esp_netif_ap = esp_netif_create_default_wifi_ap();
    //     s_esp_netif_sta = esp_netif_create_default_wifi_sta();
    //     if (s_esp_netif_ap == NULL || s_esp_netif_sta == NULL)
    //     {
    //         ESP_LOGE(TAG, "Failed to create AP/STA netifs.");
    //         HAL_WIFI_DeInit(); // Clean up
    //         return HAL_WIFI_STATUS_ERROR;
    //     }
    //     break;
    // default:
    //     // Should not happen due to initial check
    //     break;
    // }

    // ESP_ERROR_CHECK(esp_wifi_start());

    // s_wifi_initialized = true;
    // s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED; // Initial state
    // LOGI(TAG, "WiFi HAL initialized in mode %d.", mode);
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_DeInit(void)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGW(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }

    // esp_err_t ret;

    // // Stop WiFi
    // ret = esp_wifi_stop();
    // if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT)
    // {
    //     ESP_LOGE(TAG, "Failed to stop ESP WiFi: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Deinitialize WiFi
    // ret = esp_wifi_deinit();
    // if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT)
    // {
    //     ESP_LOGE(TAG, "Failed to deinitialize ESP WiFi: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Unregister event handlers
    // esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler);
    // esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler);

    // // Delete netif instances
    // if (s_esp_netif_sta)
    // {
    //     esp_netif_destroy_default_wifi_sta();
    //     s_esp_netif_sta = NULL;
    // }
    // if (s_esp_netif_ap)
    // {
    //     esp_netif_destroy_default_wifi_ap();
    //     s_esp_netif_ap = NULL;
    // }

    // // Delete the FreeRTOS event group
    // if (s_wifi_event_group != NULL)
    // {
    //     vEventGroupDelete(s_wifi_event_group);
    //     s_wifi_event_group = NULL;
    // }

    // // Deinitialize the default event loop (if it was created by us)
    // // Note: esp_event_loop_delete_default() should be called only if esp_event_loop_create_default() was called.
    // // In a real application, you might manage this more carefully.
    // esp_event_loop_delete_default();

    // // Deinitialize the network interface (TCP/IP stack)
    // esp_netif_deinit();

    // s_wifi_initialized = false;
    // s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    // s_event_callback = NULL;
    // LOGI(TAG, "WiFi HAL de-initialized successfully.");
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_Connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (s_current_wifi_state == HAL_WIFI_STATE_CONNECTED || s_current_wifi_state == HAL_WIFI_STATE_CONNECTING)
    // {
    //     ESP_LOGW(TAG, "Already connected or connecting.");
    //     return HAL_WIFI_STATUS_CONNECTED;
    // }
    // if (ssid == NULL || strlen(ssid) == 0)
    // {
    //     ESP_LOGE(TAG, "SSID cannot be NULL or empty.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // wifi_config_t wifi_config = {
    //     .sta = {
    //         .threshold.rssi = -127,                   // All RSSI
    //         .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Default to WPA2_PSK
    //         .pmf_cfg = {
    //             .required = false,
    //         },
    //     },
    // };
    // strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    // wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';

    // if (password != NULL && strlen(password) > 0)
    // {
    //     strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    //     wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';
    // }
    // else
    // {
    //     // For open networks
    //     wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    //     memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
    // }

    // esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to set WiFi STA config: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // s_retry_num = 0;
    // xEventGroupClearBits(s_wifi_event_group, (WIFI_CONNECTED_BIT | WIFI_FAIL_BIT));
    // s_current_wifi_state = HAL_WIFI_STATE_CONNECTING;

    // ret = esp_wifi_connect();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to initiate WiFi connect: %s", esp_err_to_name(ret));
    //     s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // if (timeout_ms > 0)
    // {
    //     EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
    //                                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    //                                            pdFALSE, // Don't clear bits on exit
    //                                            pdFALSE, // Wait for any bit
    //                                            pdMS_TO_TICKS(timeout_ms));

    //     if (bits & WIFI_CONNECTED_BIT)
    //     {
    //         LOGI(TAG, "Connected to AP.");
    //         return HAL_WIFI_STATUS_OK;
    //     }
    //     else if (bits & WIFI_FAIL_BIT)
    //     {
    //         ESP_LOGE(TAG, "Failed to connect to AP within timeout.");
    //         return HAL_WIFI_STATUS_ERROR; // Or more specific error like AP_NOT_FOUND, WRONG_PASSWORD
    //     }
    //     else
    //     {
    //         ESP_LOGW(TAG, "WiFi connection timed out.");
    //         return HAL_WIFI_STATUS_TIMEOUT;
    //     }
    // }

    // LOGI(TAG, "WiFi connection initiated (asynchronous).");
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_Disconnect(void)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (s_current_wifi_state == HAL_WIFI_STATE_DISCONNECTED)
    // {
    //     ESP_LOGW(TAG, "Already disconnected.");
    //     return HAL_WIFI_STATUS_OK;
    // }

    // esp_err_t ret = esp_wifi_disconnect();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to disconnect from WiFi: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }
    // s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    // xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    // LOGI(TAG, "WiFi disconnection initiated.");
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_StartAP(const char *ssid, const char *password, uint8_t channel, uint8_t max_connections, bool hidden)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (s_current_wifi_state == HAL_WIFI_STATE_AP_STARTED)
    // {
    //     ESP_LOGW(TAG, "AP already started.");
    //     return HAL_WIFI_STATUS_OK;
    // }
    // if (ssid == NULL || strlen(ssid) == 0)
    // {
    //     ESP_LOGE(TAG, "AP SSID cannot be NULL or empty.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }
    // if (channel == 0 || channel > 13)
    // {
    //     ESP_LOGE(TAG, "Invalid AP channel: %d (must be 1-13).", channel);
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }
    // if (max_connections == 0 || max_connections > 10)
    // {
    //     ESP_LOGE(TAG, "Invalid max AP connections: %d (must be 1-10).", max_connections);
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // wifi_config_t wifi_config = {
    //     .ap = {
    //         .channel = channel,
    //         .max_connection = max_connections,
    //         .authmode = WIFI_AUTH_WPA2_PSK, // Default to WPA2_PSK
    //         .ssid_hidden = hidden,
    //         .beacon_interval = 100, // Default beacon interval
    //     },
    // };
    // strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    // wifi_config.ap.ssid[sizeof(wifi_config.ap.ssid) - 1] = '\0';
    // wifi_config.ap.ssid_len = strlen((char *)wifi_config.ap.ssid);

    // if (password != NULL && strlen(password) >= 8 && strlen(password) <= 64)
    // {
    //     strncpy((char *)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
    //     wifi_config.ap.password[sizeof(wifi_config.ap.password) - 1] = '\0';
    // }
    // else if (password == NULL || strlen(password) == 0)
    // {
    //     wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    //     memset(wifi_config.ap.password, 0, sizeof(wifi_config.ap.password));
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "AP password must be between 8 and 64 characters for WPA2_PSK.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // esp_err_t ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to set WiFi AP config: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // ret = esp_wifi_start(); // esp_wifi_start() is called in HAL_WIFI_Init, but if mode changes, it might need restart
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to start WiFi AP: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // // Wait for AP_STARTED_BIT to be set (optional, as esp_wifi_start is typically synchronous for AP)
    // xEventGroupWaitBits(s_wifi_event_group, WIFI_AP_STARTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(5000)); // 5s timeout

    // LOGI(TAG, "WiFi AP '%s' started on channel %d.", ssid, channel);
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_StopAP(void)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (s_current_wifi_state != HAL_WIFI_STATE_AP_STARTED)
    // {
    //     ESP_LOGW(TAG, "AP not started.");
    //     return HAL_WIFI_STATUS_OK;
    // }

    // esp_err_t ret = esp_wifi_stop(); // Stopping WiFi will stop AP
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to stop WiFi AP: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }
    // s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    // xEventGroupClearBits(s_wifi_event_group, WIFI_AP_STARTED_BIT);
    // LOGI(TAG, "WiFi AP stopped.");
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_Scan(bool blocking)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (s_current_wifi_state == HAL_WIFI_STATE_SCANNING)
    // {
    //     ESP_LOGW(TAG, "Already scanning.");
    //     return HAL_WIFI_STATUS_BUSY;
    // }

    // s_current_wifi_state = HAL_WIFI_STATE_SCANNING;
    // s_actual_scan_aps = 0; // Clear previous scan results
    // xEventGroupClearBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);

    // wifi_scan_config_t scan_config = {
    //     .ssid = NULL,
    //     .bssid = NULL,
    //     .channel = 0,
    //     .show_hidden = true, // Show hidden SSIDs
    //     .scan_type = g_hal_wifi_config.scan_active_mode ? WIFI_SCAN_TYPE_ACTIVE : WIFI_SCAN_TYPE_PASSIVE,
    //     .scan_time.active.min = g_hal_wifi_config.scan_min_dwell_time_ms,
    //     .scan_time.active.max = g_hal_wifi_config.scan_max_dwell_time_ms,
    // };

    // esp_err_t ret = esp_wifi_scan_start(&scan_config, blocking);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to start WiFi scan: %s", esp_err_to_name(ret));
    //     s_current_wifi_state = HAL_WIFI_STATE_DISCONNECTED;
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // if (blocking)
    // {
    //     // Wait for scan to complete
    //     xEventGroupWaitBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    //     LOGI(TAG, "Blocking WiFi scan completed.");
    // }
    // else
    // {
    //     LOGI(TAG, "Asynchronous WiFi scan initiated.");
    // }

    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_GetScanResults(HAL_WIFI_AP_Info_t *ap_info_array, uint16_t max_aps, uint16_t *actual_aps)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (ap_info_array == NULL || actual_aps == NULL)
    // {
    //     ESP_LOGE(TAG, "Invalid parameters: ap_info_array or actual_aps is NULL.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // *actual_aps = 0; // Initialize
    // if (s_actual_scan_aps == 0)
    // {
    //     LOGI(TAG, "No scan results available or previous scan found no APs.");
    //     return HAL_WIFI_STATUS_OK;
    // }

    // uint16_t num_copy = (s_actual_scan_aps < max_aps) ? s_actual_scan_aps : max_aps;
    // memcpy(ap_info_array, s_scan_results, num_copy * sizeof(HAL_WIFI_AP_Info_t));
    // *actual_aps = num_copy;

    // LOGI(TAG, "Copied %d scan results.", num_copy);
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_State_t HAL_WIFI_GetState(void)
{
    // return s_current_wifi_state;
}

HAL_WIFI_Status_t HAL_WIFI_GetIPAddress(char *ip_address_buffer, uint16_t buffer_size)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (ip_address_buffer == NULL || buffer_size == 0)
    // {
    //     ESP_LOGE(TAG, "Invalid parameters: ip_address_buffer is NULL or buffer_size is 0.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // if (s_current_wifi_state != HAL_WIFI_STATE_CONNECTED)
    // {
    //     ESP_LOGW(TAG, "Not connected to an AP, no IP address available.");
    //     strncpy(ip_address_buffer, "0.0.0.0", buffer_size);
    //     ip_address_buffer[buffer_size - 1] = '\0';
    //     return HAL_WIFI_STATUS_NOT_CONNECTED;
    // }

    // esp_netif_ip_info_t ip_info;
    // esp_err_t ret = esp_netif_get_ip_info(s_esp_netif_sta, &ip_info);
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to get IP info: %s", esp_err_to_name(ret));
    //     strncpy(ip_address_buffer, "0.0.0.0", buffer_size);
    //     ip_address_buffer[buffer_size - 1] = '\0';
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // snprintf(ip_address_buffer, buffer_size, IPSTR, IP2STR(&ip_info.ip));
    // ESP_LOGD(TAG, "Retrieved IP: %s", ip_address_buffer);
    return HAL_WIFI_STATUS_OK;
}

HAL_WIFI_Status_t HAL_WIFI_GetMACAddress(uint8_t *mac_address_buffer)
{
    // if (!s_wifi_initialized)
    // {
    //     ESP_LOGE(TAG, "WiFi HAL not initialized.");
    //     return HAL_WIFI_STATUS_NOT_INITIALIZED;
    // }
    // if (mac_address_buffer == NULL)
    // {
    //     ESP_LOGE(TAG, "Invalid parameter: mac_address_buffer is NULL.");
    //     return HAL_WIFI_STATUS_INVALID_PARAM;
    // }

    // esp_err_t ret;
    // uint8_t mac[6];

    // // Get MAC address based on current mode
    // wifi_mode_t mode;
    // esp_wifi_get_mode(&mode);

    // if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
    // {
    //     ret = esp_wifi_get_mac(WIFI_IF_STA, mac);
    // }
    // else if (mode == WIFI_MODE_AP)
    // {
    //     ret = esp_wifi_get_mac(WIFI_IF_AP, mac);
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "Cannot get MAC address in current WiFi mode.");
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "Failed to get MAC address: %s", esp_err_to_name(ret));
    //     return HAL_WIFI_STATUS_ERROR;
    // }

    // memcpy(mac_address_buffer, mac, 6);
    // ESP_LOGD(TAG, "Retrieved MAC: " MACSTR, MAC2STR(mac_address_buffer));
    return HAL_WIFI_STATUS_OK;
}
