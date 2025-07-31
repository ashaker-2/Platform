// app/src/Rte.c

#include "Rte.h"
#include "logger.h"

// --- Headers for all Modules that RTE will call ---
// ECUAL Includes (for RTE_HwInitTask)
#include "ecual_gpio.h"
#include "ecual_pwm.h"
#include "ecual_adc.h"
#include "ecual_i2c.h"
#include "ecual_common.h" // For ECUAL_GetUptimeMs()

// Application Modules (for RTE_AppInitTask and RTE Services)
#include "fan.h"
#include "temp_sensor.h"
#include "ventilator.h"
#include "humidity_sensor.h"
#include "heater.h"
#include "pump.h"
#include "light_control.h"
#include "light_indication.h"
#include "character_display.h"
#include "sys_mgr.h"        // Central system manager
#include "system_monitor.h" // CPU & stack monitor

// NEW: Include the communication stack interface and middleware directly for init/task
#include "communication_stack_interface.h" // Provides the processing functions
#include "modbus_middleware.h" // Needed for init
#include "bluetooth_middleware.h" // Needed for init
#include "wifi_middleware.h" // Needed for init

// FreeRTOS Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTE";

// --- RTE Initialization Flow Tasks ---

uint8_t RTE_Init(void) {
    LOGI(TAG, "RTE_Init called. Creating initial hardware initialization task.");
    if (xTaskCreate(RTE_HwInitTask, "HwInitTask", 4096, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create HwInitTask!"); return APP_ERROR; }
    return APP_OK;
}

void RTE_HwInitTask(void *pvParameters) {
    LOGI(TAG, "RTE_HwInitTask started: Initializing ECUAL modules...");
    if (ECUAL_GPIO_Init() != ECUAL_OK) { LOGE(TAG, "GPIO ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_PWM_Init() != ECUAL_OK) { LOGE(TAG, "PWM ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_ADC_Init() != ECUAL_OK) { LOGE(TAG, "ADC ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_I2C_Init() != ECUAL_OK) { LOGE(TAG, "I2C ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    LOGI(TAG, "ECUAL Initialization complete. Creating Application Initialization Task.");
    if (xTaskCreate(RTE_AppInitTask, "AppInitTask", 4096, NULL, configMAX_PRIORITIES - 2, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create AppInitTask! Halting."); vTaskSuspend(NULL); }
    LOGI(TAG, "RTE_HwInitTask deleting itself."); vTaskDelete(NULL);
}

void RTE_AppInitTask(void *pvParameters) {
    LOGI(TAG, "RTE_AppInitTask started: Initializing Application modules...");
    if (FAN_Init() != APP_OK) { LOGE(TAG, "Fan APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (TEMP_SENSOR_Init() != APP_OK) { LOGE(TAG, "Temperature Sensor APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (VENTILATOR_Init() != APP_OK) { LOGE(TAG, "Ventilator APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (HUMIDITY_SENSOR_Init() != APP_OK) { LOGE(TAG, "Humidity Sensor APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (HEATER_Init() != APP_OK) { LOGE(TAG, "Heater APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (PUMP_Init() != APP_OK) { LOGE(TAG, "Pump APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (LIGHT_Init() != APP_OK) { LOGE(TAG, "LightControl APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (LIGHT_INDICATION_Init() != APP_OK) { LOGE(TAG, "LightIndication APP Init failed! Halting."); vTaskSuspend(NULL); }
    if (CHARACTER_DISPLAY_Init() != APP_OK) { LOGE(TAG, "CharacterDisplay APP Init failed! Halting."); vTaskSuspend(NULL); }

    // System Manager, Monitor, and Communication Initializations
    if (SYS_MGR_Init() != APP_OK) { LOGE(TAG, "System Manager Init failed! Halting."); vTaskSuspend(NULL); }
    if (SYS_MON_Init() != APP_OK) { LOGE(TAG, "System Monitor Init failed! Halting."); vTaskSuspend(NULL); }

    // NEW: Call the COMMUNICATION_STACK_Init (now defined in Rte.c)
    if (COMMUNICATION_STACK_Init() != APP_OK) { LOGE(TAG, "Communication Stack Init failed! Halting."); vTaskSuspend(NULL); }

    LOGI(TAG, "All Application modules initialized. Configuring System Manager parameters via RTE service calls...");

    // Configure System Manager using RTE service calls
    RTE_Service_SetOperationalTemperature(22.0f, 26.0f); // Desired 22-26 C
    RTE_Service_SetOperationalHumidity(45.0f, 65.0f);   // Desired 45-65 %
    RTE_Service_SetVentilatorSchedule(10, 0, 18, 0); // ON from 10:00 to 18:00 simulated time
    RTE_Service_SetLightSchedule(19, 0, 23, 0);      // ON from 19:00 to 23:00 simulated time

    // Initial display messages using RTE service calls
    RTE_Service_CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
    RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "System Init OK");
    RTE_Service_CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1);
    RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "Tasks Starting...");
    RTE_Service_CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
    RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "ALARM STATUS");
    RTE_Service_CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
    RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "Monitoring...");

    LOGI(TAG, "Calling RTE_StartAllPermanentTasks to create all permanent FreeRTOS tasks...");
    if (RTE_StartAllPermanentTasks() != APP_OK) {
        LOGE(TAG, "Failed to start all permanent tasks via RTE! Halting."); vTaskSuspend(NULL);
    }
    LOGI(TAG, "All permanent tasks created. RTE_AppInitTask deleting itself."); vTaskDelete(NULL);
}

// --- Implementation of RTE's Permanent Application Tasks ---
void RTE_SensorReadTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20);
    LOGI(TAG, "RTE_SensorReadTask started.");
    while (1) { RTE_Service_ProcessSensorReadings(); vTaskDelayUntil(&xLastWakeTime, xFrequency); }
}

void RTE_ActuatorControlTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    LOGI(TAG, "RTE_ActuatorControlTask started.");
    while (1) { RTE_Service_ControlActuators(); vTaskDelayUntil(&xLastWakeTime, xFrequency); }
}

void RTE_DisplayAlarmTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);
    LOGI(TAG, "RTE_DisplayAlarmTask started.");
    while (1) { RTE_Service_UpdateDisplayAndAlarm(); vTaskDelayUntil(&xLastWakeTime, xFrequency); }
}

void RTE_MainLoopTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // 5 second periodicity
    LOGI(TAG, "RTE_MainLoopTask started.");
    while (1) {
        RTE_Service_LIGHT_INDICATION_Toggle(LIGHT_INDICATION_SYSTEM_STATUS);
        // Example of using new WiFi RTE service from an application task:
        if (RTE_Service_WiFi_IsConnected()) {
            RTE_Service_LIGHT_INDICATION_On(LIGHT_INDICATION_WIFI_STATUS);
        } else {
            RTE_Service_LIGHT_INDICATION_Off(LIGHT_INDICATION_WIFI_STATUS);
        }

        static uint8_t backlight_toggle_counter = 0; backlight_toggle_counter++;
        if (backlight_toggle_counter % 3 == 0) {
            static bool backlight_currently_on = true;
            if(backlight_currently_on) { RTE_Service_CHARACTER_DISPLAY_BacklightOff(CHARACTER_DISPLAY_ALARM_PANEL); }
            else { RTE_Service_CHARACTER_DISPLAY_BacklightOn(CHARACTER_DISPLAY_ALARM_PANEL); }
            backlight_currently_on = !backlight_currently_on; LOGD(TAG, "Alarm Panel Backlight toggled.");
        }
        uint32_t current_hour, current_minute;
        RTE_Service_GetSimulatedTime(&current_hour, &current_minute);
        LOGV(TAG, "Main Loop: Simulated Time: %02u:%02u", current_hour, current_minute);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// NEW: Implementation of the Communication Stack Init function within RTE
uint8_t COMMUNICATION_STACK_Init(void) {
    LOGI(TAG, "Communication Stack Initialized. Initializing Middleware components...");

    // Initialize all mandatory communication middleware
    if (MODBUS_MW_Init() != APP_OK) {
        LOGE(TAG, "Modbus Middleware Init failed! Critical error.");
        return APP_ERROR;
    }
    if (BLUETOOTH_MW_Init() != APP_OK) {
        LOGE(TAG, "Bluetooth Middleware Init failed! Critical error.");
        return APP_ERROR;
    }

    // Initialize optional WiFi middleware
    if (WIFI_MW_Init() != APP_OK) {
        LOGW(TAG, "WiFi Middleware Init failed! (Optional, but noted)");
        // Continue even if optional WiFi fails, depending on system requirements
    }

    LOGI(TAG, "Communication Middleware Initialization complete.");
    return APP_OK;
}

// NEW: Implementation of the Communication Stack Main Task within RTE
void COMMUNICATION_STACK_MainTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    // Define a processing frequency. This task coordinates several communication types.
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // Process every 100ms

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "COMMUNICATION_STACK_MainTask started.");

    // Initial WiFi connection attempt (example, could be triggered by config)
    // RTE_Service_WiFi_Connect("MyHomeNetwork", "MyNetworkPassword123");
    // Note: It is generally acceptable for the task itself to call RTE services to
    // initiate actions, as long as other app modules still go through RTE services.

    while (1) {
        // --- 1. Process Modbus communication ---
        // Call the interface function, which handles MODBUS_MW_Process() and data exchange
        COMMUNICATION_STACK_ProcessModbus();

        // --- 2. Process Bluetooth communication ---
        // Call the interface function, which handles BLUETOOTH_MW_Process() and data exchange
        COMMUNICATION_STACK_ProcessBluetooth();

        // --- 3. Process WiFi communication (if enabled and connected) ---
        // Call the interface function, which handles WIFI_MW_Process() and data exchange
        COMMUNICATION_STACK_ProcessWiFi();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


// --- Function to centralize creation of ALL permanent tasks ---
uint8_t RTE_StartAllPermanentTasks(void) {
    LOGI(TAG, "RTE_StartAllPermanentTasks: Creating all permanent application tasks...");
    if (xTaskCreate(RTE_SensorReadTask, "SensorReadTask", 2048, NULL, 5, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_SensorReadTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_ActuatorControlTask, "ActuatorControlTask", 4096, NULL, 4, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_ActuatorControlTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_DisplayAlarmTask, "DisplayAlarmTask", 3072, NULL, 3, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_DisplayAlarmTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_MainLoopTask, "MainLoopTask", 2048, NULL, 2, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_MainLoopTask!"); return APP_ERROR; }
    if (xTaskCreate(SYS_MON_Task, "SysMonTask", 3072, NULL, 1, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create SYS_MON_Task!"); return APP_ERROR; }
    // The COMMUNICATION_STACK_MainTask is now explicitly created here
    if (xTaskCreate(COMMUNICATION_STACK_MainTask, "CommStackTask", 4096, NULL, 2, NULL) != pdPASS) { // Higher stack for comm
        LOGE(TAG, "Failed to create COMMUNICATION_STACK_MainTask!"); return APP_ERROR; }
    LOGI(TAG, "All permanent application tasks created successfully."); return APP_OK;
}


// --- Implementation of RTE Service Functions ---

// Services for Sensor Readings
uint8_t RTE_Service_ProcessSensorReadings(void) { return SYS_MGR_ProcessSensorReadings(); }
uint8_t RTE_Service_TEMP_SENSOR_Read(float *temperature_c) { return TEMP_SENSOR_Read(temperature_c); }
uint8_t RTE_Service_HUMIDITY_SENSOR_Read(float *humidity_p) { return HUMIDITY_SENSOR_Read(humidity_p); }

// Services for Actuator Control
uint8_t RTE_Service_ControlActuators(void) { return SYS_MGR_ControlActuators(); }
uint8_t RTE_Service_FAN_SetSpeed(uint8_t speed_percent) { return FAN_SetSpeed(speed_percent); }
uint8_t RTE_Service_HEATER_SetState(bool state) { return HEATER_SetState(state); }
uint8_t RTE_Service_PUMP_SetState(bool state) { return PUMP_SetState(state); }
uint8_t RTE_Service_VENTILATOR_SetState(bool state) { return VENTILATOR_SetState(state); }
uint8_t RTE_Service_LIGHT_SetState(uint8_t light_id, bool state) { return LIGHT_SetState(light_id, state); }
uint8_t RTE_Service_LIGHT_GetState(uint8_t light_id, bool *state) { return LIGHT_GetState(light_id, state); }

// Services for Display and Indication
uint8_t RTE_Service_UpdateDisplayAndAlarm(void) { return SYS_MGR_UpdateDisplayAndAlarm(); }
uint8_t RTE_Service_CHARACTER_DISPLAY_Clear(uint8_t display_id) { return CHARACTER_DISPLAY_Clear(display_id); }
uint8_t RTE_Service_CHARACTER_DISPLAY_PrintString(uint8_t display_id, const char *str) { return CHARACTER_DISPLAY_PrintString(display_id, str); }
uint8_t RTE_Service_CHARACTER_DISPLAY_SetCursor(uint8_t display_id, uint8_t col, uint8_t row) { return CHARACTER_DISPLAY_SetCursor(display_id, col, row); }
uint8_t RTE_Service_CHARACTER_DISPLAY_BacklightOn(uint8_t display_id) { return CHARACTER_DISPLAY_BacklightOn(display_id); }
uint8_t RTE_Service_CHARACTER_DISPLAY_BacklightOff(uint8_t display_id) { return CHARACTER_DISPLAY_BacklightOff(display_id); }
uint8_t RTE_Service_LIGHT_INDICATION_On(uint8_t indication_id) { return LIGHT_INDICATION_On(indication_id); }
uint8_t RTE_Service_LIGHT_INDICATION_Off(uint8_t indication_id) { return LIGHT_INDICATION_Off(indication_id); }
uint8_t RTE_Service_LIGHT_INDICATION_Toggle(uint8_t indication_id) { return LIGHT_INDICATION_Toggle(indication_id); }

// Services for System Manager Data Access
uint8_t RTE_Service_GetCurrentSensorReadings(float *room_temp_c, float *room_humidity_p, float *heatsink_temp_c) {
    return SYS_MGR_GetCurrentSensorReadings(room_temp_c, room_humidity_p, heatsink_temp_c); }
uint8_t RTE_Service_GetOperationalTemperature(float *min_temp_c, float *max_temp_c) {
    return SYS_MGR_GetOperationalTemperature(min_temp_c, max_temp_c); }
uint8_t RTE_Service_GetOperationalHumidity(float *min_humidity_p, float *max_humidity_p) {
    return SYS_MGR_GetOperationalHumidity(min_humidity_p, max_humidity_p); }
uint8_t RTE_Service_GetActuatorStates(uint8_t *fan_stage, uint64_t *last_fan_time_ms, bool *heater_working, bool *pump_working, bool *ventilator_working, bool *fan_any_active) {
    return SYS_MGR_GetActuatorStates(fan_stage, last_fan_time_ms, heater_working, pump_working, ventilator_working, fan_any_active); }
uint8_t RTE_Service_GetSimulatedTime(uint32_t *hour, uint32_t *minute) {
    return SYS_MGR_GetSimulatedTime(hour, minute); }
uint8_t RTE_Service_SetOperationalTemperature(float min_temp_c, float max_temp_c) {
    return SYS_MGR_SetOperationalTemperature(min_temp_c, max_temp_c); }
uint8_t RTE_Service_SetOperationalHumidity(float min_humidity_p, float max_humidity_p) {
    return SYS_MGR_SetOperationalHumidity(min_humidity_p, max_humidity_p); }
uint8_t RTE_Service_SetVentilatorSchedule(uint32_t start_hour, uint32_t start_minute, uint32_t end_hour, uint32_t end_minute) {
    return SYS_MGR_SetVentilatorSchedule(start_hour, start_minute, end_hour, end_minute); }
uint8_t RTE_Service_SetLightSchedule(uint32_t start_hour, uint32_t start_minute, uint32_t end_hour, uint32_t end_minute) {
    return SYS_MGR_SetLightSchedule(start_hour, start_minute, end_hour, end_minute); }

// Services for System Monitor Data Access
uint8_t RTE_Service_GetCPULoad(uint8_t *cpu_load_percent) {
    if (cpu_load_percent == NULL) return APP_ERROR;
    *cpu_load_percent = SYS_MON_GetCPULoad(); return APP_OK; }
uint8_t RTE_Service_GetTotalMinFreeStack(uint32_t *total_min_free_stack_bytes) {
    if (total_min_free_stack_bytes == NULL) return APP_ERROR;
    *total_min_free_stack_bytes = SYS_MON_GetTotalMinFreeStack(); return APP_OK; }


// --- Implementation of NEW RTE Services for Communication ---
// These functions act as a bridge from any application module to the COMMUNICATION_STACK_INTERFACE's internal handlers.

uint8_t RTE_Service_Modbus_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value) {
    return COMMUNICATION_STACK_Internal_Modbus_WriteHoldingRegister(slave_address, register_address, value);
}

uint8_t RTE_Service_Modbus_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value) {
    return COMMUNICATION_STACK_Internal_Modbus_ReadHoldingRegister(slave_address, register_address, value);
}

uint8_t RTE_Service_Bluetooth_SendData(uint16_t characteristic_uuid, const uint8_t *data, uint16_t len) {
    return COMMUNICATION_STACK_Internal_Bluetooth_SendData(characteristic_uuid, data, len);
}

uint8_t RTE_Service_WiFi_Connect(const char *ssid, const char *password) {
    return COMMUNICATION_STACK_Internal_WiFi_Connect(ssid, password);
}

uint8_t RTE_Service_WiFi_Disconnect(void) {
    return COMMUNICATION_STACK_Internal_WiFi_Disconnect();
}

bool RTE_Service_WiFi_IsConnected(void) {
    return COMMUNICATION_STACK_Internal_WiFi_IsConnected();
}

uint8_t RTE_Service_WiFi_SendNetworkData(const char *endpoint, const uint8_t *data, uint16_t len) {
    return COMMUNICATION_STACK_Internal_WiFi_SendNetworkData(endpoint, data, len);
}
