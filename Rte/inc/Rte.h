// app/inc/Rte.h
#ifndef RTE_H
#define RTE_H

#include <stdint.h>
#include <stdbool.h> // For bool type
#include "app_common.h"     // For APP_OK/APP_ERROR
#include "freertos/FreeRTOS.h" // For TaskFunction_t (FreeRTOS task prototype)

// --- Initialization Functions and Tasks ---
uint8_t RTE_Init(void);
void RTE_HwInitTask(void *pvParameters);
void RTE_AppInitTask(void *pvParameters);
uint8_t RTE_StartAllPermanentTasks(void);


// --- Permanent Application Tasks (Implementations in Rte.c) ---
void RTE_SensorReadTask(void *pvParameters);
void RTE_ActuatorControlTask(void *pvParameters);
void RTE_DisplayAlarmTask(void *pvParameters);
void RTE_MainLoopTask(void *pvParameters);

// NEW: Communication Stack Task and Init function, now managed directly by RTE
/**
 * @brief Initialize the entire Communication Stack.
 * This function sets up the underlying Modbus, Bluetooth, and WiFi middleware components.
 * Called by RTE_AppInitTask.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t COMMUNICATION_STACK_Init(void);

/**
 * @brief Main Communication Stack Task.
 * This FreeRTOS task orchestrates all communication activities.
 * It periodically calls the processing functions for Modbus, Bluetooth, and WiFi.
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void COMMUNICATION_STACK_MainTask(void *pvParameters);


// --- RTE Service Functions (for inter-component calls) ---
// These functions are the only allowed way for modules to interact with each other.

// Services for Sensor Readings
uint8_t RTE_Service_ProcessSensorReadings(void);
uint8_t RTE_Service_TEMP_SENSOR_Read(float *temperature_c);
uint8_t RTE_Service_HUMIDITY_SENSOR_Read(float *humidity_p);


// Services for Actuator Control
uint8_t RTE_Service_ControlActuators(void);
uint8_t RTE_Service_FAN_SetSpeed(uint8_t speed_percent);
uint8_t RTE_Service_HEATER_SetState(bool state);
uint8_t RTE_Service_PUMP_SetState(bool state);
uint8_t RTE_Service_VENTILATOR_SetState(bool state);
uint8_t RTE_Service_LIGHT_SetState(uint8_t light_id, bool state);
uint8_t RTE_Service_LIGHT_GetState(uint8_t light_id, bool *state);


// Services for Display and Indication
uint8_t RTE_Service_UpdateDisplayAndAlarm(void);
uint8_t RTE_Service_CHARACTER_DISPLAY_Clear(uint8_t display_id);
uint8_t RTE_Service_CHARACTER_DISPLAY_PrintString(uint8_t display_id, const char *str);
uint8_t RTE_Service_CHARACTER_DISPLAY_SetCursor(uint8_t display_id, uint8_t col, uint8_t row);
uint8_t RTE_Service_CHARACTER_DISPLAY_BacklightOn(uint8_t display_id);
uint8_t RTE_Service_CHARACTER_DISPLAY_BacklightOff(uint8_t display_id);
uint8_t RTE_Service_LIGHT_INDICATION_On(uint8_t indication_id);
uint8_t RTE_Service_LIGHT_INDICATION_Off(uint8_t indication_id);
uint8_t RTE_Service_LIGHT_INDICATION_Toggle(uint8_t indication_id);


// Services for System Manager Data Access
uint8_t RTE_Service_GetCurrentSensorReadings(float *room_temp_c, float *room_humidity_p, float *heatsink_temp_c);
uint8_t RTE_Service_GetOperationalTemperature(float *min_temp_c, float *max_temp_c);
uint8_t RTE_Service_GetOperationalHumidity(float *min_humidity_p, float *max_humidity_p);
uint8_t RTE_Service_GetActuatorStates(uint8_t *fan_stage, uint64_t *last_fan_time_ms, bool *heater_working, bool *pump_working, bool *ventilator_working, bool *fan_any_active);
uint8_t RTE_Service_GetSimulatedTime(uint32_t *hour, uint32_t *minute);
uint8_t RTE_Service_SetOperationalTemperature(float min_temp_c, float max_temp_c);
uint8_t RTE_Service_SetOperationalHumidity(float min_humidity_p, float max_humidity_p);
uint8_t RTE_Service_SetVentilatorSchedule(uint32_t start_hour, uint32_t start_minute, uint32_t end_hour, uint32_t end_minute);
uint8_t RTE_Service_SetLightSchedule(uint32_t start_hour, uint32_t start_minute, uint32_t end_hour, uint32_t end_minute);


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
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t RTE_Service_Modbus_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value);

/**
 * @brief RTE Service to read a value from a Modbus holding register.
 * This function will be called by any application component that needs to read from a Modbus register.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param slave_address The Modbus slave ID.
 * @param register_address The 16-bit address of the holding register.
 * @param value Pointer to a uint16_t where the read value will be stored.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t RTE_Service_Modbus_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value);

/**
 * @brief RTE Service to send data over Bluetooth.
 * This function will be called by any application component that needs to send data via Bluetooth.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param characteristic_uuid The UUID of the GATT characteristic to send data through.
 * @param data Pointer to the data to send.
 * @param len Length of the data in bytes.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t RTE_Service_Bluetooth_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len);

/**
 * @brief RTE Service to initiate WiFi connection.
 * This function will be called by any application component that needs to manage WiFi connection.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @param ssid The SSID of the access point.
 * @param password The password for the access point.
 * @return APP_OK if connection initiated, APP_ERROR otherwise.
 */
uint8_t RTE_Service_WiFi_Connect(const char *ssid, const char *password);

/**
 * @brief RTE Service to disconnect from WiFi.
 * This function will be called by any application component that needs to manage WiFi connection.
 * It routes the call to the COMMUNICATION_STACK_INTERFACE's internal handler.
 * @return APP_OK if successful, APP_ERROR otherwise.
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
 * @return APP_OK if send operation is initiated, APP_ERROR otherwise.
 */
uint8_t RTE_Service_WiFi_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len);


#endif /* RTE_H */
