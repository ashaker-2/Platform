// app/src/communication_stack_interface.c

#include "communication_stack_interface.h"
#include "logger.h"

// This module includes the middleware directly as it provides the interface to it.
#include "modbus_middleware.h"
#include "bluetooth_middleware.h"
#include "wifi_middleware.h"

// This module interacts with other application components ONLY through RTE.
#include "Rte.h" // For RTE_Service_ calls

static const char *TAG = "COMM_STACK_INTF"; // Interface Tag

// --- PRIVATE HELPER FUNCTIONS (only callable within this .c file) ---

/**
 * @brief Handles data exchange for Modbus.
 * This includes:
 * 1. Reading application data via RTE services and writing to Modbus registers (for master to read).
 * 2. Reading Modbus registers (that master might have written) and updating SYS_MGR via RTE services.
 */
static void communication_stack_process_modbus_data_exchange(void) {
    float room_temp, room_hum, heatsink_temp;
    uint8_t fan_stage;
    uint64_t last_fan_time_ms; // Not directly used in Modbus reg, but part of data struct
    bool heater_working, pump_working, ventilator_working, fan_any_active;
    uint16_t value_read;

    // --- Update Modbus registers with current system status (for Modbus master to read) ---
    if (RTE_Service_GetCurrentSensorReadings(&room_temp, &room_hum, &heatsink_temp) == APP_OK) {
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_ROOM_TEMP_X100, (uint16_t)(room_temp * 100));
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_HUMIDITY_X100, (uint16_t)(room_hum * 100));
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_HEATSINK_TEMP_X100, (uint16_t)(heatsink_temp * 100));
    }

    if (RTE_Service_GetActuatorStates(&fan_stage, &last_fan_time_ms, &heater_working, &pump_working, &ventilator_working, &fan_any_active) == APP_OK) {
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_FAN_STAGE, fan_stage);
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_HEATER_STATE, heater_working ? 1 : 0);
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_PUMP_STATE, pump_working ? 1 : 0);
        MODBUS_MW_WriteHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_VENTILATOR_STATE, ventilator_working ? 1 : 0);
    }

    // --- Read Modbus registers for commands/settings from Modbus master ---
    // Example: Check if Modbus master updated Min Operational Temperature
    if (MODBUS_MW_ReadHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_SET_MIN_OP_TEMP_X100, &value_read) == APP_OK) {
        float current_min_temp, current_max_temp;
        RTE_Service_GetOperationalTemperature(&current_min_temp, &current_max_temp); // Get current settings
        float new_min_temp = (float)value_read / 100.0f;
        if (new_min_temp != current_min_temp) {
            LOGI(TAG, "Modbus: Detected new Min Op Temp: %.2fC. Updating SYS_MGR.", new_min_temp);
            RTE_Service_SetOperationalTemperature(new_min_temp, current_max_temp);
        }
    }

    // Example: Check if Modbus master updated Max Operational Temperature
    if (MODBUS_MW_ReadHoldingRegister(MODBUS_SLAVE_ID, MODBUS_REG_SET_MAX_OP_TEMP_X100, &value_read) == APP_OK) {
        float current_min_temp, current_max_temp;
        RTE_Service_GetOperationalTemperature(&current_min_temp, &current_max_temp);
        float new_max_temp = (float)value_read / 100.0f;
        if (new_max_temp != current_max_temp) {
            LOGI(TAG, "Modbus: Detected new Max Op Temp: %.2fC. Updating SYS_MGR.", new_max_temp);
            RTE_Service_SetOperationalTemperature(current_min_temp, new_max_temp);
        }
    }
    // Add similar logic for humidity and other writable registers
}

/**
 * @brief Handles data exchange for Bluetooth.
 * This includes:
 * 1. Sending periodic sensor/status data via Bluetooth.
 * 2. Checking for incoming Bluetooth commands (if a receive mechanism exists).
 */
static void communication_stack_process_bluetooth_data_exchange(void) {
    float room_temp, room_hum;
    uint8_t fan_stage;
    bool heater_working, pump_working, ventilator_working;
    uint8_t cpu_load;
    bool light_state = false; // Placeholder for light state

    // Get current system data via RTE Services
    RTE_Service_GetCurrentSensorReadings(&room_temp, &room_hum, NULL); // Heatsink temp not included in this BT example
    RTE_Service_GetActuatorStates(&fan_stage, NULL, &heater_working, &pump_working, &ventilator_working, NULL);
    RTE_Service_GetCPULoad(&cpu_load);
    RTE_Service_LIGHT_GetState(0, &light_state); // Assuming LIGHT_ID_MAIN_ROOM is 0

    // Format data into a string payload
    char bt_payload[128];
    snprintf(bt_payload, sizeof(bt_payload), "T:%.1f,H:%.1f,F:%u,H:%u,P:%u,V:%u,L:%u,C:%u",
             room_temp, room_hum, fan_stage, heater_working, pump_working, ventilator_working, light_state, cpu_load);

    // Send the data via Bluetooth Middleware
    BLUETOOTH_MW_SendData(BT_CHAR_SENSOR_DATA_UUID, (const uint8_t*)bt_payload, strlen(bt_payload));

    // Example: Sending an alarm status if an alarm is active (assuming an RTE service for alarm status)
    // if (RTE_Service_GetAlarmStatus()) {
    //    BLUETOOTH_MW_SendData(BT_CHAR_ALARM_STATUS_UUID, (const uint8_t*)"ALARM_ACTIVE", strlen("ALARM_ACTIVE"));
    // }
}

/**
 * @brief Handles data exchange for WiFi.
 * This includes:
 * 1. Checking WiFi connectivity status.
 * 2. Sending periodic data to a cloud service (e.g., HTTP POST, MQTT publish).
 */
static void communication_stack_process_wifi_data_exchange(void) {
    if (WIFI_MW_IsConnected()) {
        float room_temp, room_hum;
        uint8_t cpu_load;
        uint32_t min_free_stack;

        RTE_Service_GetCurrentSensorReadings(&room_temp, &room_hum, NULL);
        RTE_Service_GetCPULoad(&cpu_load);
        RTE_Service_GetTotalMinFreeStack(&min_free_stack);

        char json_payload[256];
        snprintf(json_payload, sizeof(json_payload),
                 "{\"temp\":%.1f,\"humidity\":%.1f,\"cpu_load\":%u,\"min_stack\":%u}",
                 room_temp, room_hum, cpu_load, min_free_stack);

        // Example: Send data to a mock cloud endpoint
        WIFI_MW_SendNetworkData("http://api.myiotplatform.com/data", (const uint8_t*)json_payload, strlen(json_payload));
    } else {
        LOGD(TAG, "WiFi is not connected. Attempting to reconnect...");
        // This could be made more sophisticated with retry logic, backoff, etc.
        // For now, it will just keep trying to connect at init.
        // WIFI_MW_Connect("MyHomeNetwork", "MyNetworkPassword123"); // Re-attempt connection if needed
    }
}


// --- PUBLIC FUNCTIONS (exposed via communication_stack_interface.h for COMMUNICATION_STACK_MainTask) ---

uint8_t COMMUNICATION_STACK_ProcessModbus(void) {
    MODBUS_MW_Process();
    communication_stack_process_modbus_data_exchange();
    return APP_OK;
}

uint8_t COMMUNICATION_STACK_ProcessBluetooth(void) {
    BLUETOOTH_MW_Process();
    communication_stack_process_bluetooth_data_exchange();
    return APP_OK;
}

uint8_t COMMUNICATION_STACK_ProcessWiFi(void) {
    WIFI_MW_Process();
    communication_stack_process_wifi_data_exchange();
    return APP_OK;
}

// --- INTERNAL FUNCTIONS (called by RTE Services) ---

uint8_t COMMUNICATION_STACK_Internal_Modbus_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value) {
    return MODBUS_MW_WriteHoldingRegister(slave_address, register_address, value);
}

uint8_t COMMUNICATION_STACK_Internal_Modbus_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value) {
    return MODBUS_MW_ReadHoldingRegister(slave_address, register_address, value);
}

uint8_t COMMUNICATION_STACK_Internal_Bluetooth_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len) {
    return BLUETOOTH_MW_SendData(characteristic_uuid, data, len);
}

uint8_t COMMUNICATION_STACK_Internal_WiFi_Connect(const char *ssid, const char *password) {
    return WIFI_MW_Connect(ssid, password);
}

uint8_t COMMUNICATION_STACK_Internal_WiFi_Disconnect(void) {
    return WIFI_MW_Disconnect();
}

bool COMMUNICATION_STACK_Internal_WiFi_IsConnected(void) {
    return WIFI_MW_IsConnected();
}

uint8_t COMMUNICATION_STACK_Internal_WiFi_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len) {
    return WIFI_MW_SendNetworkData(endpoint, data, len);
}
