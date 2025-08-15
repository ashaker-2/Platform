// app/inc/communication_stack_interface.h
#ifndef COMMUNICATION_STACK_INTERFACE_H
#define COMMUNICATION_STACK_INTERFACE_H

#include <stdint.h>
#include <stdbool.h> // For bool
#include "common.h" // For E_OK/E_NOK types
#include "modbus_middleware.h" // For Modbus register definitions, etc.
#include "bluetooth_middleware.h" // For Bluetooth characteristic UUIDs, etc.
#include "wifi_middleware.h" // For WiFi endpoint definitions, etc.

// --- Internal functions exposed for use by the ComM_MainTask in Rte.c ---
// These functions directly interact with the underlying middleware.
// They are NOT meant to be called directly by other application modules.
// Application modules must use the RTE_Service_COMM_... functions.

/**
 * @brief Performs periodic processing for the Modbus stack.
 * Calls MODBUS_MW_Process() and handles data exchange.
 * @return E_OK if processing is successful, E_NOK if an unrecoverable error occurs.
 */
uint8_t COMMUNICATION_STACK_ProcessModbus(void);

/**
 * @brief Performs periodic processing for the Bluetooth stack.
 * Calls BLUETOOTH_MW_Process() and handles data exchange.
 * @return E_OK if processing is successful, E_NOK if an unrecoverable error occurs.
 */
uint8_t COMMUNICATION_STACK_ProcessBluetooth(void);

/**
 * @brief Performs periodic processing for the WiFi stack.
 * Calls WIFI_MW_Process() and handles data exchange.
 * @return E_OK if processing is successful, E_NOK if an unrecoverable error occurs.
 */
uint8_t COMMUNICATION_STACK_ProcessWiFi(void);


// --- Internal functions to be called ONLY by RTE_Service_COMM_... functions ---
// These provide a layer of indirection for RTE to use to communicate with the stack.

/**
 * @brief Internal function to write a value to a Modbus holding register.
 * Called by RTE_Service_Modbus_WriteHoldingRegister.
 * @param slave_address The Modbus slave ID.
 * @param register_address The 16-bit address of the holding register.
 * @param value The 16-bit value to write.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_Modbus_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value);

/**
 * @brief Internal function to read a value from a Modbus holding register.
 * Called by RTE_Service_Modbus_ReadHoldingRegister.
 * @param slave_address The Modbus slave ID.
 * @param register_address The 16-bit address of the holding register.
 * @param value Pointer to a uint16_t where the read value will be stored.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_Modbus_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value);

/**
 * @brief Internal function to send data over Bluetooth.
 * Called by RTE_Service_Bluetooth_SendData.
 * @param characteristic_uuid The UUID of the GATT characteristic.
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_Bluetooth_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len);

/**
 * @brief Internal function to initiate WiFi connection.
 * Called by RTE_Service_WiFi_Connect.
 * @param ssid The SSID of the access point.
 * @param password The password for the access point.
 * @return E_OK if connection initiated, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_WiFi_Connect(const char *ssid, const char *password);

/**
 * @brief Internal function to disconnect from WiFi.
 * Called by RTE_Service_WiFi_Disconnect.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_WiFi_Disconnect(void);

/**
 * @brief Internal function to get WiFi connection status.
 * Called by RTE_Service_WiFi_IsConnected.
 * @return True if connected, false otherwise.
 */
bool COMMUNICATION_STACK_Internal_WiFi_IsConnected(void);

/**
 * @brief Internal function to send data over network (e.g. HTTP, MQTT).
 * Called by RTE_Service_WiFi_SendNetworkData.
 * @param endpoint Network endpoint (e.g. URL, MQTT topic).
 * @param data Data to send.
 * @param len Length of data.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t COMMUNICATION_STACK_Internal_WiFi_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len);


#endif /* COMMUNICATION_STACK_INTERFACE_H */
