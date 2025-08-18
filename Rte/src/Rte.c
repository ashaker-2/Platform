// app/src/Rte.c

#include "Rte.h"
#include "logger.h"

// --- Headers for all Modules that RTE will call ---
// HAL Includes (for RTE_HwInitTask)
#include "hal_gpio.h"
#include "hal_pwm.h"
#include "hal_adc.h"
#include "hal_i2c.h"
#include "common.h" // For HAL_GetUptimeMs()

// Application Modules (for RTE_AppInitTask and RTE Services)
#include "fanctrl.h"
#include "tempctrl.h"
#include "ventctrl.h"
#include "humctrl.h"
#include "Heaterctrl.h"
#include "pumpctrl.h"
#include "lightctrl.h"
#include "lightind.h"
// #include "char_display.h"
#include "sys_mgr.h"        // Central system manager
#include "system_monitor.h" // CPU & stack monitor

// NEW: Include the communication stack interface and middleware directly for init/task
// #include "com.h" // Provides the processing functions
// #include "modbus.h" // Needed for init

// FreeRTOS Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTE";

// --- RTE Initialization Flow Tasks ---

uint8_t RTE_Init(void)
{
    // LOGI(TAG, "RTE_Init called. Creating initial hardware initialization task.");
    if (xTaskCreate(RTE_HwInitTask, "HwInitTask", 256, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create HwInitTask!"); return E_NOK;
    }
    return E_OK;
}

void RTE_HwInitTask(void *pvParameters)
{
    // LOGI(TAG, "RTE_HwInitTask started: Initializing HAL modules...");
    if (HAL_GPIO_Init() != E_OK)
    {
        // // LOGE(TAG, "GPIO HAL Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (HAL_PWM_Init() != E_OK)
    {
        // // LOGE(TAG, "PWM HAL Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (HAL_ADC_Init() != E_OK)
    {
        // // LOGE(TAG, "ADC HAL Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    // if (HAL_I2C_Init() != E_OK)
    // {
    //     // // LOGE(TAG, "I2C HAL Init failed! Halting.");
    //     vTaskSuspend(NULL);
    // }

    // LOGI(TAG, "HAL Initialization complete. Creating Application Initialization Task.");
    if (xTaskCreate(RTE_AppInitTask, "AppInitTask", 4096, NULL, configMAX_PRIORITIES - 2, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create AppInitTask! Halting.");
        vTaskSuspend(NULL);
    }
    // LOGI(TAG, "RTE_HwInitTask deleting itself.");
    vTaskDelete(NULL);
}

void RTE_AppInitTask(void *pvParameters)
{
    // LOGI(TAG, "RTE_AppInitTask started: Initializing Application modules...");
    if (FanCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Fan APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (TempCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Temperature Sensor APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (VentCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Ventilator APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (HumCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Humidity Sensor APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (HeaterCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Heater APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (PumpCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "Pump APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (LightCtrl_Init() != E_OK)
    {
        // // LOGE(TAG, "LightControl APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (LightInd_Init() != E_OK)
    {
        // // LOGE(TAG, "LightIndication APP Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    // if (CHARACTER_DISPLAY_Init() != E_OK)
    // {
    //     // // LOGE(TAG, "CharacterDisplay APP Init failed! Halting.");
    //     vTaskSuspend(NULL);
    // }

    // System Manager, Monitor, and Communication Initializations
    if (SYS_MGR_Init() != E_OK)
    {
        // // LOGE(TAG, "System Manager Init failed! Halting.");
        vTaskSuspend(NULL);
    }
    if (SysMon_Init() != E_OK)
    {
        // // LOGE(TAG, "System Monitor Init failed! Halting.");
        vTaskSuspend(NULL);
    }

    // NEW: Call the ComM_Init (now defined in Rte.c)
    if (ComM_Init() != E_OK)
    {
        // // LOGE(TAG, "Communication Stack Init failed! Halting.");
        vTaskSuspend(NULL);
    }

    // LOGI(TAG, "All Application modules initialized. Configuring System Manager parameters via RTE service calls...");

    // Configure System Manager using RTE service calls
    // RTE_Service_SetOperationalTemperature(22.0f, 26.0f); // Desired 22-26 C
    // RTE_Service_SetOperationalHumidity(45.0f, 65.0f);   // Desired 45-65 %
    // RTE_Service_SetVentilatorSchedule(10, 0, 18, 0); // ON from 10:00 to 18:00 simulated time
    // RTE_Service_SetLightSchedule(19, 0, 23, 0);      // ON from 19:00 to 23:00 simulated time

    // // Initial display messages using RTE service calls
    // RTE_Service_CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
    // RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "System Init OK");
    // RTE_Service_CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1);
    // RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "Tasks Starting...");
    // RTE_Service_CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
    // RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "ALARM STATUS");
    // RTE_Service_CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
    // RTE_Service_CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "Monitoring...");

    // LOGI(TAG, "Calling RTE_StartAllPermanentTasks to create all permanent FreeRTOS tasks...");
    if (RTE_StartAllPermanentTasks() != E_OK)
    {
        // LOGE(TAG, "Failed to start all permanent tasks via RTE! Halting.");
        vTaskSuspend(NULL);
    }
    // LOGI(TAG, "All permanent tasks created. RTE_AppInitTask deleting itself.");
    vTaskDelete(NULL);
}

// --- Implementation of RTE's Permanent Application Tasks ---
void TaskAppCore0_20ms_Pri_3(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20);
    // LOGI(TAG, "TaskAppCore0_20ms_Pri_3 started.");
    while (1)
    {

        TempCtrl_MainFunction();
        HumCtrl_MainFunction();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_100ms_Pri_3(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    // LOGI(TAG, "TaskAppCore0_100ms_Pri_3 started.");
    while (1)
    {
        SYS_MGR_MainFunction();
        SysMon_MainFunction();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_150ms_Pri_4(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);
    // LOGI(TAG, "TaskAppCore0_150ms_Pri_4 started.");
    while (1)
    {
        // RTE_Service_UpdateDisplayAndAlarm();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_200ms_Pri_5(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // 5 second periodicity
    // LOGI(TAG, "TaskAppCore0_200ms_Pri_5 started.");
    while (1)
    {
        uint32_t current_hour, current_minute;
        // RTE_Service_GetSimulatedTime(&current_hour, &current_minute);
        // LOGV(TAG, "Main Loop: Simulated Time: %02u:%02u", current_hour, current_minute);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// NEW: Implementation of the Communication Stack Init function within RTE
uint8_t ComM_Init(void)
{
    // LOGI(TAG, "Communication Stack Initialized. Initializing Middleware components...");

    // // Initialize all mandatory communication middleware
    // if (MODBUS_MW_Init() != E_OK)
    // {
    //     // // LOGE(TAG, "Modbus Middleware Init failed! Critical error.");
    //     return E_NOK;
    // }
    // if (BLUETOOTH_MW_Init() != E_OK)
    // {
    //     // // LOGE(TAG, "Bluetooth Middleware Init failed! Critical error.");
    //     return E_NOK;
    // }

    // // Initialize optional WiFi middleware
    // if (WIFI_MW_Init() != E_OK)
    //    {
    //     // // LOGW(TAG, "WiFi Middleware Init failed! (Optional, but noted)");
    //     // Continue even if optional WiFi fails, depending on system requirements
    // }

    // LOGI(TAG, "Communication Middleware Initialization complete.");
    return E_OK;
}

// NEW: Implementation of the Communication Stack Main Task within RTE
void TaskAppCore1_50ms_Pri_2(void *pvParameters)
{
    TickType_t xLastWakeTime;
    // Define a processing frequency. This task coordinates several communication types.
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // Process every 100ms

    xLastWakeTime = xTaskGetTickCount();
    // LOGI(TAG, "TaskAppCore1_50ms_Pri_2 started.");

    // Initial WiFi connection attempt (example, could be triggered by config)
    // RTE_Service_WiFi_Connect("MyHomeNetwork", "MyNetworkPassword123");
    // Note: It is generally acceptable for the task itself to call RTE services to
    // initiate actions, as long as other app modules still go through RTE services.

    while (1)
    {
        // --- 1. Process Modbus communication ---
        // Call the interface function, which handles MODBUS_MW_Process() and data exchange
        // COMMUNICATION_STACK_ProcessModbus();

        // --- 2. Process Bluetooth communication ---
        // Call the interface function, which handles BLUETOOTH_MW_Process() and data exchange
        // COMMUNICATION_STACK_ProcessBluetooth();

        // --- 3. Process WiFi communication (if enabled and connected) ---
        // Call the interface function, which handles WIFI_MW_Process() and data exchange
        // COMMUNICATION_STACK_ProcessWiFi();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// --- Function to centralize creation of ALL permanent tasks ---
uint8_t RTE_StartAllPermanentTasks(void)
{
    // LOGI(TAG, "RTE_StartAllPermanentTasks: Creating all permanent application tasks...");
    if (xTaskCreate(TaskAppCore0_20ms_Pri_3, "TaskAppCore0_20ms_Pri_3", 2048, NULL, 5, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create TaskAppCore0_20ms_Pri_3!");
        return E_NOK;
    }
    if (xTaskCreate(TaskAppCore0_100ms_Pri_3, "TaskAppCore0_100ms_Pri_3", 4096, NULL, 4, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create TaskAppCore0_100ms_Pri_3!");
        return E_NOK;
    }
    if (xTaskCreate(TaskAppCore0_150ms_Pri_4, "TaskAppCore0_150ms_Pri_4", 3072, NULL, 3, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create TaskAppCore0_150ms_Pri_4!");
        return E_NOK;
    }
    if (xTaskCreate(TaskAppCore0_200ms_Pri_5, "TaskAppCore0_200ms_Pri_5", 2048, NULL, 2, NULL) != pdPASS)
    {
        // // LOGE(TAG, "Failed to create TaskAppCore0_200ms_Pri_5!");
        return E_NOK;
    }

    // The TaskAppCore1_50ms_Pri_2 is now explicitly created here
    if (xTaskCreate(TaskAppCore1_50ms_Pri_2, "TaskAppCore1_50ms_Pri_2", 4096, NULL, 2, NULL) != pdPASS)
    {
        // Higher stack for comm
        // // LOGE(TAG, "Failed to create TaskAppCore1_50ms_Pri_2!");
        return E_NOK;
    }
    // LOGI(TAG, "All permanent application tasks created successfully.");
    return E_OK;
}

// --- Implementation of RTE Service Functions ---

// Services for Sensor Readings

// Services for System Monitor Data Access
uint8_t RTE_Service_GetCPULoad(uint8_t *cpu_load_percent)
{
    if (cpu_load_percent == NULL)
    {
        return E_NOK;
    }
    // *cpu_load_percent = SYS_MON_GetCPULoad();
    return E_OK;
}

uint8_t RTE_Service_GetTotalMinFreeStack(uint32_t *total_min_free_stack_bytes)
{
    if (total_min_free_stack_bytes == NULL)
    {
        return E_NOK;
    }
    // *total_min_free_stack_bytes = SYS_MON_GetTotalMinFreeStack();
    return E_OK;
}
