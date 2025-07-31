// app/middleware/inc/wifi_middleware.h
#ifndef WIFI_MIDDLEWARE_H
#define WIFI_MIDDLEWARE_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h" // For APP_OK/APP_ERROR

/**
 * @brief Initialize the WiFi middleware.
 * This sets up the WiFi stack (e.g., ESP-IDF WiFi).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t WIFI_MW_Init(void);

/**
 * @brief Connects to a specified WiFi access point.
 * @param ssid The SSID (network name) of the access point.
 * @param password The password for the access point.
 * @return APP_OK if connection initiated successfully, APP_ERROR otherwise.
 */
uint8_t WIFI_MW_Connect(const char *ssid, const char *password);

/**
 * @brief Disconnects from the current WiFi access point.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t WIFI_MW_Disconnect(void);

/**
 * @brief Gets the current WiFi connection status.
 * @return True if connected to an AP and has an IP address, false otherwise.
 */
bool WIFI_MW_IsConnected(void);

/**
 * @brief Sends data over a network connection (e.g., HTTP POST, MQTT publish).
 * This is a placeholder for application-level network communication.
 * @param endpoint A string representing the destination (e.g., URL, MQTT topic).
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return APP_OK if send operation is initiated, APP_ERROR otherwise.
 */
uint8_t WIFI_MW_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len);

/**
 * @brief Performs periodic processing for the WiFi stack.
 * This function would typically handle network events, keep-alives,
 * and manage queued network operations. It is intended to be called frequently by COMM_MainTask.
 * @return APP_OK if processing is successful, APP_ERROR if an unrecoverable error occurs.
 */
uint8_t WIFI_MW_Process(void);

#endif // WIFI_MIDDLEWARE_H
