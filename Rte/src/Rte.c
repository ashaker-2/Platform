// app/src/Rte.c

#include "Rte.h"
#include "logger.h"             // For logging
#include "sys_mgr.h"            // To call SYS_MGR logic functions
#include "ecual_common.h"       // For ECUAL_GetUptimeMs()
#include "light_indication.h"   // For heartbeat LED in MainLoopTask
#include "character_display.h"  // For backlight control in MainLoopTask

// Headers for tasks whose implementations are in other files
#include "init_tasks.h"     // For INIT_TASKS_HwInitTask, INIT_TASKS_AppInitTask
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
    if (xTaskCreate(INIT_TASKS_HwInitTask, "InitHwTask", 4096, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create InitHwTask!");
        return APP_ERROR;
    }
    return APP_OK;
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
        
        // WiFi Status LED (always ON for this demo)
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

// --- New function to centralize creation of ALL permanent tasks ---
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
