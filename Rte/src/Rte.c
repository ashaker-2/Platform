// app/src/Rte.c

#include "Rte.h"
#include "logger.h"             // For logging
#include "sys_mgr.h"            // To call SYS_MGR logic functions
#include "ecual_common.h"       // For ECUAL_GetUptimeMs()
#include "light_indication.h"   // For heartbeat LED in MainLoopTask
#include "character_display.h"  // For backlight control in MainLoopTask

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTE";

// Forward declarations of internal SYS_MGR logic functions (exposed by sys_mgr.c)
// These are not in sys_mgr.h because they are only intended to be called by RTE tasks.
// They will be defined in sys_mgr.c and will handle mutex protection internally.
extern void SYS_MGR_ProcessSensorReadings(void);
extern void SYS_MGR_ControlActuators(void);
extern void SYS_MGR_UpdateDisplayAndAlarm(void);


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
