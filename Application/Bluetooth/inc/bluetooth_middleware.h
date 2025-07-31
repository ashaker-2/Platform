// app/middleware/inc/bluetooth_middleware.h
#ifndef BLUETOOTH_MIDDLEWARE_H
#define BLUETOOTH_MIDDLEWARE_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h" // For APP_OK/APP_ERROR

// Example service and characteristic UUIDs (placeholders, use actual UUIDs in a real BLE app)
#define BT_SERVICE_SYSTEM_INFO_UUID   0x180A // Device Information Service
#define BT_CHAR_SENSOR_DATA_UUID      0x2A19 // Battery Level (mocked for sensor data)
#define BT_CHAR_COMMANDS_UUID         0x2A06 // Alert Level (mocked for commands to device)
#define BT_CHAR_ALARM_STATUS_UUID     0x2A00 // Generic Access Profile (mocked for alarms)


/**
 * @brief Initialize the Bluetooth middleware.
 * This sets up the Bluetooth host and controller (e.g., ESP-IDF NimBLE/Bluedroid stack).
 * It typically involves configuring GAP (advertising) and GATT services.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t BLUETOOTH_MW_Init(void);

/**
 * @brief Sends data over Bluetooth (e.g., as a GATT notification/indication).
 * This function simulates sending data through a specific characteristic.
 * @param characteristic_uuid The UUID of the GATT characteristic to send data through.
 * @param data Pointer to the data buffer to send.
 * @param len Length of the data in bytes.
 * @return APP_OK if the send operation is successful, APP_ERROR otherwise.
 */
uint8_t BLUETOOTH_MW_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len);

/**
 * @brief Performs periodic processing for the Bluetooth stack.
 * This function would typically handle queued advertising updates, connection events,
 * and incoming GATT write requests. It is intended to be called frequently by COMM_MainTask.
 * @return APP_OK if processing is successful, APP_ERROR if an unrecoverable error occurs.
 */
uint8_t BLUETOOTH_MW_Process(void);

#endif // BLUETOOTH_MIDDLEWARE_H
