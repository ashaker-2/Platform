// main/startup.c

// ECUAL Includes
#include "ecual_gpio.h"
#include "ecual_pwm.h"
#include "ecual_adc.h"
#include "ecual_i2c.h"
#include "ecual_uart.h"
#include "ecual_common.h" // For ECUAL_GetUptimeMs()

// Application Modules Includes
#include "fan.h"
#include "temp_sensor.h"
#include "ventilator.h"
#include "humidity_sensor.h"
#include "heater.h"
#include "pump.h"
#include "light_control.h"
#include "light_indication.h"
#include "character_display.h"
#include "logger.h"
#include "sys_mgr.h"

// Rte (Runtime Environment) Tasks Includes
#include "Rte.h" // New header for RTE tasks

// FreeRTOS Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Standard C Library Includes
#include <stdio.h>

static const char *APP_TAG = "STARTUP";

/**
 * @brief Main application entry point.
 * This function handles the overall system initialization and starts the FreeRTOS scheduler.
 */
void app_main(void) {
    // --- 1. MACAL (Microcontroller Abstraction Layer) & ECUAL Initialization ---
    // On ESP-IDF, much of the low-level MACAL initialization is handled by the ESP-IDF framework
    // before app_main is called. We focus on ECUAL here.

    // Initialize ECUAL UART first, as the Logger depends on it.
    // Ensure ECUAL_UART_Init() is configured for the console UART (e.g., ECUAL_UART_0).
    if (ECUAL_UART_Init() != ECUAL_OK) { 
        // If UART fails, we can't even log properly. Use direct printf as a last resort.
        printf("CRITICAL ERROR: UART ECUAL Init failed! System cannot proceed.\r\n");
        // In a production system, this would lead to a system reset or safe mode.
        return; 
    }
    
    // Initialize the generic Logger. From this point, use LOGx macros.
    if (LOGGER_Init() != APP_OK) { 
        LOGE(APP_TAG, "Logger Init failed! Serial logging may not work reliably."); 
        // Decide if this is a critical error for your application.
        // For this demo, we'll continue, but logging might be impaired.
    }
    
    LOGI(APP_TAG, "Initializing ECUAL modules...");
    if (ECUAL_GPIO_Init() != ECUAL_OK) { LOGE(APP_TAG, "GPIO ECUAL Init failed!"); return; }
    if (ECUAL_PWM_Init() != ECUAL_OK) { LOGE(APP_TAG, "PWM ECUAL Init failed!"); return; }
    if (ECUAL_ADC_Init() != ECUAL_OK) { LOGE(APP_TAG, "ADC ECUAL Init failed!"); return; }
    if (ECUAL_I2C_Init() != ECUAL_OK) { LOGE(APP_TAG, "I2C ECUAL Init failed!"); return; }

    // --- 2. Application Modules Initialization ---
    LOGI(APP_TAG, "Initializing Application modules...");
    if (FAN_Init() != APP_OK) { LOGE(APP_TAG, "Fan APP Init failed!"); return; }
    if (TEMP_SENSOR_Init() != APP_OK) { LOGE(APP_TAG, "Temperature Sensor APP Init failed!"); return; }
    if (VENTILATOR_Init() != APP_OK) { LOGE(APP_TAG, "Ventilator APP Init failed!"); return; }
    if (HUMIDITY_SENSOR_Init() != APP_OK) { LOGE(APP_TAG, "Humidity Sensor APP Init failed!"); return; }
    if (HEATER_Init() != APP_OK) { LOGE(APP_TAG, "Heater APP Init failed!"); return; }
    if (PUMP_Init() != APP_OK) { LOGE(APP_TAG, "Pump APP Init failed!"); return; }
    if (LIGHT_Init() != APP_OK) { LOGE(APP_TAG, "LightControl APP Init failed!"); return; }
    if (LIGHT_INDICATION_Init() != APP_OK) { LOGE(APP_TAG, "LightIndication APP Init failed!"); return; }
    if (CHARACTER_DISPLAY_Init() != APP_OK) { LOGE(APP_TAG, "CharacterDisplay APP Init failed!"); return; }

    // Initialize System Manager (This will set up its internal state and mutex)
    if (SYS_MGR_Init() != APP_OK) {
        LOGE(APP_TAG, "System Manager Init failed! Demo logic will not run.");
        return;
    }

    // --- Optional: Configure System Manager from "User Input" (simulated here) ---
    // These calls will update the SYS_MGR's internal state, which tasks will then read.
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

    // --- 3. Start the Tasks (RTE Tasks) ---
    LOGI(APP_TAG, "Creating and starting FreeRTOS tasks...");

    // Sensor Reading Task (20ms periodicity)
    if (xTaskCreate(RTE_SensorReadTask, "SensorReadTask", 2048, NULL, 5, NULL) != pdPASS) {
        LOGE(APP_TAG, "Failed to create SensorReadTask!");
        return;
    }
    // Actuator Control Task (100ms periodicity)
    if (xTaskCreate(RTE_ActuatorControlTask, "ActuatorControlTask", 4096, NULL, 4, NULL) != pdPASS) {
        LOGE(APP_TAG, "Failed to create ActuatorControlTask!");
        return;
    }
    // Display & Alarm Task (1000ms periodicity)
    if (xTaskCreate(RTE_DisplayAlarmTask, "DisplayAlarmTask", 3072, NULL, 3, NULL) != pdPASS) {
        LOGE(APP_TAG, "Failed to create DisplayAlarmTask!");
        return;
    }
    // Main Loop Task (5000ms periodicity) - This is what the original app_main loop becomes
    if (xTaskCreate(RTE_MainLoopTask, "MainLoopTask", 2048, NULL, 2, NULL) != pdPASS) {
        LOGE(APP_TAG, "Failed to create MainLoopTask!");
        return;
    }

    // Start the FreeRTOS scheduler
    LOGI(APP_TAG, "Scheduler starting...");
    vTaskStartScheduler();

    // The code below this line will only be reached if there's an error in the scheduler
    // or if vTaskEndScheduler() is called (which is rare in embedded systems).
    LOGE(APP_TAG, "Scheduler exited unexpectedly!");
    while (1) {
        // Infinite loop to prevent returning from app_main if scheduler fails
        // In a real system, you might trigger a watchdog reset here.
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
