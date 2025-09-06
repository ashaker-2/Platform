// app/src/Rte.c

#include "Rte.h"
#include "logger.h"

// --- Headers for all Modules that RTE will call ---
// HAL Includes (for RTE_HwInitTask)
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_adc.h"
#include "hal_i2c.h"
#include "common.h" // For HAL_GetUptimeMs()

// Application Modules (for RTE_AppInitTask and RTE Services)
#include "fanctrl.h"
#include "temphumctrl.h"
#include "venctrl.h"
#include "Heaterctrl.h"
#include "pumpctrl.h"
#include "lightctrl.h"
#include "ledctrl.h"
#include "char_display.h"
#include "sys_mgr.h"        // Central system manager
#include "system_monitor.h" // CPU & stack monitor
#include "hal_init.h"
// NEW: Include the communication stack interface and middleware directly for init/task
// #include "com.h" // Provides the processing functions

// FreeRTOS Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CORE0 0
// only define xCoreID CORE1 as 1 if this is a multiple core processor target, else define it as tskNO_AFFINITY
#define CORE1 ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)

static const char *TAG = "RTE";

// --- RTE Initialization Flow Tasks ---

uint8_t RTE_Init(void)
{
    if (xTaskCreatePinnedToCore(RTE_HwInitTask, "HwInitTask", 4096, NULL, configMAX_PRIORITIES - 1, NULL, tskNO_AFFINITY) != pdPASS)
    {
        LOGE(TAG, "Failed to create HwInitTask!");
        return E_NOK;
    }

    if (xTaskCreatePinnedToCore(RTE_AppInitTask, "AppInitTask", 4096, NULL, configMAX_PRIORITIES - 2, NULL, tskNO_AFFINITY) != pdPASS)
    {
        LOGE(TAG, "Failed to create AppInitTask! Halting.");
        return E_NOK;
    }

    return E_OK;
}

void RTE_HwInitTask(void *pvParameters)
{
    LOGI(TAG, "RTE_HwInitTask started: Initializing HAL modules...");
    if (HAL_Init() != E_OK)
    {
        LOGE(TAG, "GPIO HAL Init failed! Halting.");
        vTaskSuspend(NULL);
    }

    LOGI(TAG, "HAL Initialization complete.");

    LOGI(TAG, "RTE_HwInitTask deleting itself.");
    vTaskDelete(NULL);
}

void RTE_AppInitTask(void *pvParameters)
{
    LOGI(TAG, "RTE_AppInitTask started: Initializing Application modules...");
    if (FanCtrl_Init() != E_OK)
    {
        LOGE(TAG, "Fan APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (TempHumCtrl_Init() != E_OK)
    {
        LOGE(TAG, "Temperature and Humidity Sensor APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (VenCtrl_Init() != E_OK)
    {
        LOGE(TAG, "Ventilator APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (HeaterCtrl_Init() != E_OK)
    {
        LOGE(TAG, "Heater APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (PumpCtrl_Init() != E_OK)
    {
        LOGE(TAG, "Pump APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (LightCtrl_Init() != E_OK)
    {
        LOGE(TAG, "LightControl APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (LedCtrl_Init() != E_OK)
    {
        LOGE(TAG, "LightIndication APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (HAL_CharDisplay_Init() != E_OK)
    {
        LOGE(TAG, "CharacterDisplay APP Init failed! Halting.");
        // vTaskSuspend(NULL);
    }

    // NEW: Call the ComM_Init (now defined in Rte.c)
    if (ComM_Init() != E_OK)
    {
        LOGE(TAG, "Communication Stack Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    // System Manager, Monitor, and Communication Initializations
    if (SystemMonitor_Init() != E_OK)
    {
        LOGE(TAG, "System Monitor Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    if (SYS_MGR_Init() != E_OK)
    {
        LOGE(TAG, "System Manager Init failed! Halting.");
        // vTaskSuspend(NULL);
    }
    LOGI(TAG, "All Application modules initialized. Configuring System Manager parameters via RTE service calls...");

    LOGI(TAG, "Calling RTE_StartAllPermanentTasks to create all permanent FreeRTOS tasks...");
    if (RTE_StartAllPermanentTasks() != E_OK)
    {
        LOGE(TAG, "Failed to start all permanent tasks via RTE! Halting.");
        vTaskSuspend(NULL);
    }
    LOGI(TAG, "All permanent tasks created. RTE_AppInitTask deleting itself.");
    vTaskDelete(NULL);
}

// --- Implementation of RTE's Permanent Application Tasks ---
void TaskAppCore0_20ms_Pri_3(void *pvParameters)
{
    int task_id = (int)(intptr_t)pvParameters; // recover the int
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20);
    LOGI(TAG, "Task Id : %d TaskAppCore0_20ms_Pri_3 started.", task_id);
    while (1)
    {

        TempHumCtrl_MainFunction();
        // LOGI(TAG, "Hi from TaskAppCore0_20ms_Pri_3!");
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_100ms_Pri_3(void *pvParameters)
{
    int task_id = (int)(intptr_t)pvParameters; // recover the int
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    LOGI(TAG, "Task Id : %d TaskAppCore0_100ms_Pri_3 started.", task_id);
    while (1)
    {
        
        SysMon_MainFunction();
        // LOGI(TAG, "Hi from TaskAppCore0_100ms_Pri_3!");
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_150ms_Pri_4(void *pvParameters)
{
    int task_id = (int)(intptr_t)pvParameters; // recover the int
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(150);
    LOGI(TAG, "Task Id : %d TaskAppCore0_150ms_Pri_4 started.", task_id);
    while (1)
    {
        SYS_MGR_MainFunction();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void TaskAppCore0_200ms_Pri_5(void *pvParameters)
{
    int task_id = (int)(intptr_t)pvParameters; // recover the int
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(200); // 5 second periodicity
    LOGI(TAG, "Task Id : %d TaskAppCore0_200ms_Pri_5 started.", task_id);
    while (1)
    {
        SYS_MGR_MainFunction();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// NEW: Implementation of the Communication Stack Main Task within RTE
void TaskAppCore1_50ms_Pri_2(void *pvParameters)
{
    int task_id = (int)(intptr_t)pvParameters; // recover the int
    TickType_t xLastWakeTime;
    // Define a processing frequency. This task coordinates several communication types.
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // Process every 100ms

    xLastWakeTime = xTaskGetTickCount();
    LOGI(TAG, "Task Id : %d TaskAppCore1_50ms_Pri_2 started.", task_id);

    // Initial WiFi connection attempt (example, could be triggered by config)
    // RTE_Service_WiFi_Connect("MyHomeNetwork", "MyNetworkPassword123");
    // Note: It is generally acceptable for the task itself to call RTE services to
    // initiate actions, as long as other app modules still go through RTE services.

    while (1)
    {
        // LOGI(TAG, "Hi from TaskAppCore1_50ms_Pri_2!");
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

    uint8_t task_id0 = 0;
    uint8_t task_id1 = 1;
    uint8_t task_id2 = 2;
    uint8_t task_id3 = 3;
    uint8_t task_id4 = 4;

    LOGI(TAG, "RTE_StartAllPermanentTasks: Creating all permanent application tasks...");

    if (xTaskCreatePinnedToCore(TaskAppCore0_20ms_Pri_3, "TaskAppCore0_20ms_Pri_3", 4096, (void *)(intptr_t)task_id0, 5, NULL, CORE0) != pdPASS)
    {
        LOGE(TAG, "Failed to create TaskAppCore0_20ms_Pri_3!");
        return E_NOK;
    }

    if (xTaskCreatePinnedToCore(TaskAppCore0_100ms_Pri_3, "TaskAppCore0_100ms_Pri_3", 4096, (void *)(intptr_t)task_id1, 4, NULL, CORE0) != pdPASS)
    {
        LOGE(TAG, "Failed to create TaskAppCore0_100ms_Pri_3!");
        return E_NOK;
    }

    if (xTaskCreatePinnedToCore(TaskAppCore0_150ms_Pri_4, "TaskAppCore0_150ms_Pri_4", 4096, (void *)(intptr_t)task_id2, 4, NULL, CORE0) != pdPASS)
    {
        LOGE(TAG, "Failed to create TaskAppCore0_150ms_Pri_4!");
        return E_NOK;
    }

    if (xTaskCreatePinnedToCore(TaskAppCore0_200ms_Pri_5, "TaskAppCore0_200ms_Pri_5", 4096, (void *)(intptr_t)task_id3, 6, NULL, CORE0) != pdPASS)
    {
        LOGE(TAG, "Failed to create TaskAppCore0_200ms_Pri_5!");
        return E_NOK;
    }

    // The TaskAppCore1_50ms_Pri_2 is now explicitly created here
    if (xTaskCreatePinnedToCore(TaskAppCore1_50ms_Pri_2, "TaskAppCore1_50ms_Pri_2", 4096, (void *)(intptr_t)task_id4, 7, NULL, CORE1) != pdPASS)
    {
        // Higher stack for comm
        LOGE(TAG, "Failed to create TaskAppCore1_50ms_Pri_2!");
        return E_NOK;
    }
    LOGI(TAG, "All permanent application tasks created successfully.");
    return E_OK;
}

// --- Implementation of RTE Service Functions ---

// NEW: Implementation of the Communication Stack Init function within RTE
uint8_t ComM_Init(void)
{
    // LOGI(TAG, "Communication Stack Initialized. Initializing Middleware components...");

    // Initialize all mandatory communication middleware
    // if (MODBUS_MW_Init() != E_OK)
    // {
    //     LOGE(TAG, "Modbus Middleware Init failed! Critical error.");
    //     return E_NOK;
    // }
    // if (BLUETOOTH_MW_Init() != E_OK)
    // {
    //     LOGE(TAG, "Bluetooth Middleware Init failed! Critical error.");
    //     return E_NOK;
    // }

    // Initialize optional WiFi middleware
    // if (WIFI_MW_Init() != E_OK)
    //    {
    //     LOGW(TAG, "WiFi Middleware Init failed! (Optional, but noted)");
    //     // Continue even if optional WiFi fails, depending on system requirements
    // }

    // LOGI(TAG, "Communication Middleware Initialization complete.");
    return E_OK;
}

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
