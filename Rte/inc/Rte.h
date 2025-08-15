// app/inc/Rte.h
#ifndef RTE_H
#define RTE_H

#include <stdint.h>
#include <stdbool.h> // For bool type
#include "common.h"     // For E_OK/E_NOK
#include "FreeRTOS.h" // For TaskFunction_t (FreeRTOS task prototype)

// --- Initialization Functions and Tasks ---
uint8_t RTE_Init(void);
void RTE_HwInitTask(void *pvParameters);
void RTE_AppInitTask(void *pvParameters);
uint8_t RTE_StartAllPermanentTasks(void);


// --- Permanent System Tasks (Implementations in Rte.c) ---
void TaskAppCore0_20ms_Pri_3(void *pvParameters);
void TaskAppCore0_100ms_Pri_3(void *pvParameters);
void TaskAppCore0_150ms_Pri_4(void *pvParameters);
void TaskAppCore0_200ms_Pri_5(void *pvParameters);
void TaskAppCore1_50ms_Pri_2(void *pvParameters);

// NEW: Communication Stack Task and Init function, now managed directly by RTE
/**
 * @brief Initialize the entire Communication Stack.
 * This function sets up the underlying Modbus, Bluetooth, and WiFi middleware components.
 * Called by RTE_AppInitTask.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t ComM_Init(void);

/**
 * @brief Main Communication Stack Task.
 * This FreeRTOS task orchestrates all communication activities.
 * It periodically calls the processing functions for Modbus, Bluetooth, and WiFi.
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void ComM_MainTask(void *pvParameters);



// Services for System Monitor Data Access
uint8_t RTE_Service_GetCPULoad(uint8_t *cpu_load_percent);
uint8_t RTE_Service_GetTotalMinFreeStack(uint32_t *total_min_free_stack_bytes);


// --- RTE Services for Communication (calling COMMUNICATION_STACK_INTERFACE's internal functions) ---

/**
 * @brief RTE Service to write a value to a Modbus holding register.
 * This function will be called by any application component that needs to write to a Modbus register.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param slave_address The Modbus slave ID.
 * @param register_address The 16-bit address of the holding register.
 * @param value The 16-bit value to write.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t RTE_Service_Modbus_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value);

/**
 * @brief RTE Service to read a value from a Modbus holding register.
 * This function will be called by any application component that needs to read from a Modbus register.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param slave_address The Modbus slave ID.
 * @param register_address The 16-bit address of the holding register.
 * @param value Pointer to a uint16_t where the read value will be stored.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t RTE_Service_Modbus_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value);

/**
 * @brief RTE Service to send data over Bluetooth.
 * This function will be called by any application component that needs to send data via Bluetooth.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param characteristic_uuid The UUID of the GATT characteristic to send data through.
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t RTE_Service_Bluetooth_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len);

/**
 * @brief RTE Service to initiate WiFi connection.
 * This function will be called by any application component that needs to manage WiFi connection.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param ssid The SSID of the access point.
 * @param password The password for the access point.
 * @return E_OK if connection initiated, E_NOK otherwise.
 */
uint8_t RTE_Service_WiFi_Connect(const char *ssid, const char *password);

/**
 * @brief RTE Service to disconnect from WiFi.
 * This function will be called by any application component that needs to manage WiFi connection.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t RTE_Service_WiFi_Disconnect(void);

/**
 * @brief RTE Service to get WiFi connection status.
 * This function will be called by any application component that needs to know WiFi status.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @return True if connected, false otherwise.
 */
bool RTE_Service_WiFi_IsConnected(void);

/**
 * @brief RTE Service to send data over a network connection (e.g., HTTP POST, MQTT publish).
 * This function will be called by any application component that needs to send network data.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param endpoint A string representing the destination (e.g., URL, MQTT topic).
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return E_OK if send operation is initiated, E_NOK otherwise.
 */
uint8_t RTE_Service_WiFi_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len);


#endif /* RTE_H */
