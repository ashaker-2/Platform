// app/src/Rte.c

#include "Rte.h"
#include "logger.h"             // For logging
#include "sys_mgr.h"            // To call SYS_MGR logic functions
#include "ecual_common.h"       // For ECUAL_GetUptimeMs()
#include "light_indication.h"   // For heartbeat LED in MainLoopTask
#include "character_display.h"  // For backlight control in MainLoopTask

// ECUAL Includes (now directly included here for RTE_HwInitTask)
#include "ecual_gpio.h"
#include "ecual_pwm.h"
#include "ecual_adc.h"
#include "ecual_i2c.h"

// Application Modules Includes (now directly included here for RTE_AppInitTask)
#include "fan.h"
#include "temp_sensor.h"
#include "ventilator.h"
#include "humidity_sensor.h"
#include "heater.h"
#include "pump.h"
#include "light_control.h"
#include "light_indication.h"
#include "character_display.h"

// Headers for tasks whose implementations are in other files (permanent tasks)
#include "system_monitor.h" // For SYS_MON_Task
#include "comm_task.h"      // For COMM_MainTask

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTE";

// Forward declarations of internal SYS_MGR logic functions (exposed by sys_mgr.c)
extern void SYS_MGR_ProcessSensorReadings(void);
extern void SYS_MGR_ControlActuators(void);
extern void SYS_MGR_UpdateDisplayAndAlarm(void);


uint8_t RTE_Init(void) {
    LOGI(TAG, "RTE_Init called. Creating initial hardware initialization task.");

    // Create the initial hardware initialization task
    if (xTaskCreate(RTE_HwInitTask, "HwInitTask", 4096, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create HwInitTask!");
        return APP_ERROR;
    }
    return APP_OK;
}

// --- Implementation of the Initialization Tasks (now part of RTE) ---

void RTE_HwInitTask(void *pvParameters) {
    LOGI(TAG, "RTE_HwInitTask started: Initializing ECUAL modules...");

    // ECUAL Initializations
    if (ECUAL_GPIO_Init() != ECUAL_OK) { LOGE(TAG, "GPIO ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_PWM_Init() != ECUAL_OK) { LOGE(TAG, "PWM ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_ADC_Init() != ECUAL_OK) { LOGE(TAG, "ADC ECUAL Init failed! Halting."); vTaskSuspend(NULL); }
    if (ECUAL_I2C_Init() != ECUAL_OK) { LOGE(TAG, "I2C ECUAL Init failed! Halting."); vTaskSuspend(NULL); }

    LOGI(TAG, "ECUAL Initialization complete. Creating Application Initialization Task.");

    // Create the Application Initialization Task
    if (xTaskCreate(RTE_AppInitTask, "AppInitTask", 4096, NULL, configMAX_PRIORITIES - 2, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create AppInitTask! Halting.");
        vTaskSuspend(NULL); // Suspend self if unable to create next task
    }

    // Delete this task as its job is done
    LOGI(TAG, "RTE_HwInitTask deleting itself.");
    vTaskDelete(NULL);
}


void RTE_AppInitTask(void *pvParameters) {
    LOGI(TAG, "RTE_AppInitTask started: Initializing Application modules...");

    // Application Modules Initializations
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
    if (COMM_Init() != APP_OK) { LOGE(TAG, "Communication Task Init failed! Halting."); vTaskSuspend(NULL); }

    LOGI(TAG, "All Application modules initialized. Configuring System Manager parameters...");

    // Configure System Manager from "User Input" (simulated here)
    SYS_MGR_SetOperationalTemperature(22.0f, 26.0f); // Desired 22-26 C
    SYS_MGR_SetOperationalHumidity(45.0f, 65.0f);   // Desired 45-65 %
    SYS_MGR_SetVentilatorSchedule(10, 0, 18, 0); // ON from 10:00 to 18:00 simulated time
    SYS_MGR_SetLightSchedule(19, 0, 23, 0);      // ON from 19:00 to 23:00 simulated time

    // Initial display messages (tasks will update these shortly)
    CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "System Init OK");
    CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "Tasks Starting...");

    CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "ALARM STATUS");
    CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "Monitoring...");

    LOGI(TAG, "Calling RTE_StartAllPermanentTasks to create all permanent FreeRTOS tasks...");
    // This function contains ALL xTaskCreate calls for the ongoing application tasks.
    if (RTE_StartAllPermanentTasks() != APP_OK) {
        LOGE(TAG, "Failed to start all permanent tasks via RTE! Halting.");
        vTaskSuspend(NULL);
    }

    LOGI(TAG, "All permanent tasks created. RTE_AppInitTask deleting itself.");
    vTaskDelete(NULL); // Delete this task as its job is done
}

// --- Implementation of RTE's permanent tasks (as before) ---

void RTE_SensorReadTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms periodicity

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "RTE_SensorReadTask started.");

    while (1) {
        SYS_MGR_ProcessSensorReadings(); // Call SYS_MGR for sensor logic
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void RTE_ActuatorControlTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms periodicity

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "RTE_ActuatorControlTask started.");

    while (1) {
        SYS_MGR_ControlActuators(); // Call SYS_MGR for actuator control logic
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void RTE_DisplayAlarmTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second periodicity

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "RTE_DisplayAlarmTask started.");

    while (1) {
        SYS_MGR_UpdateDisplayAndAlarm(); // Call SYS_MGR for display and alarm logic
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void RTE_MainLoopTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // 5 second periodicity

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "RTE_MainLoopTask started.");

    while (1) {
        // System Status LED heartbeat
        LIGHT_INDICATION_Toggle(LIGHT_INDICATION_SYSTEM_STATUS);
        
        // WiFi Status LED (always ON for this demo, would be dynamic in a real system)
        LIGHT_INDICATION_On(LIGHT_INDICATION_WIFI_STATUS);

        // Toggle backlight for Alarm Panel every 15 seconds
        static uint8_t backlight_toggle_counter = 0;
        backlight_toggle_counter++;
        if (backlight_toggle_counter % 3 == 0) { // Toggle every 3 * 5s = 15s
            static bool backlight_currently_on = true;
            if(backlight_currently_on) {
                CHARACTER_DISPLAY_BacklightOff(CHARACTER_DISPLAY_ALARM_PANEL);
            } else {
                CHARACTER_DISPLAY_BacklightOn(CHARACTER_DISPLAY_ALARM_PANEL);
            }
            backlight_currently_on = !backlight_currently_on;
            LOGD(TAG, "Alarm Panel Backlight toggled.");
        }
        
        uint32_t current_hour, current_minute;
        SYS_MGR_GetSimulatedTime(&current_hour, &current_minute);
        LOGV(TAG, "Main Loop: Simulated Time: %02u:%02u", current_hour, current_minute);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- Function to centralize creation of ALL permanent tasks ---
uint8_t RTE_StartAllPermanentTasks(void) {
    LOGI(TAG, "RTE_StartAllPermanentTasks: Creating all permanent application tasks...");

    // RTE Tasks (Core Application Logic) - implementations are in this file (Rte.c)
    if (xTaskCreate(RTE_SensorReadTask, "SensorReadTask", 2048, NULL, 5, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_SensorReadTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_ActuatorControlTask, "ActuatorControlTask", 4096, NULL, 4, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_ActuatorControlTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_DisplayAlarmTask, "DisplayAlarmTask", 3072, NULL, 3, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_DisplayAlarmTask!"); return APP_ERROR; }
    if (xTaskCreate(RTE_MainLoopTask, "MainLoopTask", 2048, NULL, 2, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create RTE_MainLoopTask!"); return APP_ERROR; }

    // System Monitoring Task - implementation in system_monitor.c
    if (xTaskCreate(SYS_MON_Task, "SysMonTask", 3072, NULL, 1, NULL) != pdPASS) { // Lowest priority
        LOGE(TAG, "Failed to create SYS_MON_Task!"); return APP_ERROR; }

    // Communication Task (Modbus and Bluetooth) - implementation in comm_task.c
    if (xTaskCreate(COMM_MainTask, "CommTask", 4096, NULL, 2, NULL) != pdPASS) { // Medium priority
        LOGE(TAG, "Failed to create COMM_MainTask!"); return APP_ERROR; }

    LOGI(TAG, "All permanent application tasks created successfully.");
    return APP_OK;
}
