#include "system_monitor.h"
#include "app_common.h"
#include "logger.h"
#include "Rte.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

/**
 * @file system_monitor.c
 * @brief Implementation for the SystemMonitor (Fault Manager) component.
 *
 * This file contains the core logic for managing system faults and health metrics,
 * as detailed in the SystemMonitor Detailed Design Document.
 */

// --- Internal State Variables ---

// Circular buffer for historical faults
static SystemMonitor_FaultRecord_t s_historical_fault_log[SysMon_HISTORY_SIZE];
static uint32_t s_history_write_idx = 0;
static uint32_t s_history_count = 0;

// Array for active faults
static SystemMonitor_FaultRecord_t s_active_faults[SysMon_MAX_ACTIVE_FAULTS];
static uint32_t s_active_fault_count = 0;

static uint8_t  s_current_cpu_load_percent = 0;
static uint32_t s_total_min_free_stack_bytes = 0;
static bool     s_is_initialized = false;

// Mutex to protect access to internal fault logs and metrics
static SemaphoreHandle_t s_system_monitor_mutex;

// --- Private Helper Function Prototypes ---
static void SystemMonitor_AddHistoricalFault(const SystemMonitor_FaultRecord_t *new_fault);
static void SystemMonitor_AddActiveFault(const SystemMonitor_FaultRecord_t *new_fault);

// --- Public Function Implementations ---

APP_Status_t SysMon_Init(void) {
    if (s_is_initialized) {
        return APP_OK; // Already initialized
    }

    // Create the mutex to protect internal data
    s_system_monitor_mutex = xSemaphoreCreateMutex();
    if (s_system_monitor_mutex == NULL) {
        // Log to a basic output if possible, as the logger may not be fully initialized
        // This is a critical failure.
        return APP_ERROR;
    }

    // Initialize all state variables under mutex protection
    xSemaphoreTake(s_system_monitor_mutex, portMAX_DELAY);
    
    s_history_write_idx = 0;
    s_history_count = 0;
    memset(s_historical_fault_log, 0, sizeof(s_historical_fault_log));

    s_active_fault_count = 0;
    memset(s_active_faults, 0, sizeof(s_active_faults));

    s_current_cpu_load_percent = 0;
    s_total_min_free_stack_bytes = (uint32_t)-1; // Initialized to max value
    
    xSemaphoreGive(s_system_monitor_mutex);
    
    s_is_initialized = true;
    LOGI("SystemMonitor", "Module initialized successfully.");
    return APP_OK;
}

APP_Status_t SysMon_ReportFault(uint32_t fault_id,
                                SystemMonitor_FaultSeverity_t severity,
                                uint32_t data) {
    if (!s_is_initialized) {
        // Can't report, so this is a fatal error
        return APP_ERROR;
    }

    // Validate inputs against the configured fault IDs
    if (fault_id >= SysMon_TOTAL_FAULT_IDS || severity >= SEVERITY_NONE) {
        LOGE("SystemMonitor", "Invalid fault_id (%lu) or severity (%u) reported.", fault_id, severity);
        return APP_ERROR;
    }

    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        LOGE("SystemMonitor", "Failed to acquire mutex for fault report.");
        return APP_ERROR;
    }

    SystemMonitor_FaultRecord_t new_fault = {
        .id = fault_id,
        .severity = severity,
        .timestamp_ms = APP_COMMON_GetUptimeMs(),
        .data = data,
        .is_active = true
    };

    // Add fault to historical log
    SystemMonitor_AddHistoricalFault(&new_fault);

    // Add/Update fault in the active fault list
    SystemMonitor_AddActiveFault(&new_fault);

    // Log the fault based on severity
    if (severity >= SEVERITY_HIGH) {
        LOGE("SystemMonitor", "CRITICAL FAULT: ID %lu, Data %lu", fault_id, data);
    } else if (severity >= SEVERITY_MEDIUM) {
        LOGW("SystemMonitor", "Warning FAULT: ID %lu, Data %lu", fault_id, data);
    } else {
        LOGI("SystemMonitor", "Info FAULT: ID %lu, Data %lu", fault_id, data);
    }

    xSemaphoreGive(s_system_monitor_mutex);
    return APP_OK;
}

void SysMon_MainFunction(void) {
    if (!s_is_initialized) {
        return;
    }
    
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    uint32_t ulTotalRunTime;

    // Get number of tasks to allocate memory for the array
    uxArraySize = uxTaskGetNumberOfTasks();
    if (uxArraySize == 0) {
        return;
    }

    // Allocate memory for the task status array
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
    if (pxTaskStatusArray == NULL) {
        LOGE("SystemMonitor", "Failed to allocate memory for task status.");
        return;
    }

    // Get task status and total runtime
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);
    
    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        vPortFree(pxTaskStatusArray);
        return;
    }

    // Reset stack monitoring value to find the new minimum
    s_total_min_free_stack_bytes = (uint32_t)-1;
    
    if (ulTotalRunTime > 0) {
        for (x = 0; x < uxArraySize; x++) {
            // Monitor stack HWM
            if (pxTaskStatusArray[x].usStackHighWaterMark * sizeof(StackType_t) < s_total_min_free_stack_bytes) {
                s_total_min_free_stack_bytes = pxTaskStatusArray[x].usStackHighWaterMark * sizeof(StackType_t);
            }
        }
    }
    
    // Check for critical active faults and request fail-safe mode
    for (uint32_t i = 0; i < s_active_fault_count; i++) {
        if (s_active_faults[i].severity >= SEVERITY_HIGH) {
            RTE_Service_SYS_MGR_SetFailSafeMode(true);
            break; // Only need to request once
        }
    }
    
    LOGI("SystemMonitor", "CPU Load: %u%%, Min Free Stack: %lu bytes",
         s_current_cpu_load_percent, s_total_min_free_stack_bytes);
         
    vPortFree(pxTaskStatusArray);
    xSemaphoreGive(s_system_monitor_mutex);
}

uint8_t SysMon_GetCPULoad(void) {
    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return 0;
    }
    uint8_t load = s_current_cpu_load_percent;
    xSemaphoreGive(s_system_monitor_mutex);
    return load;
}

uint32_t SysMon_GetTotalMinFreeStack(void) {
    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return 0;
    }
    uint32_t stack = s_total_min_free_stack_bytes;
    xSemaphoreGive(s_system_monitor_mutex);
    return stack;
}

APP_Status_t SysMon_GetFaultStatus(SystemMonitor_FaultStatus_t *status) {
    if (status == NULL) {
        return APP_ERROR;
    }
    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return APP_ERROR;
    }

    status->active_fault_count = s_active_fault_count;
    status->historical_fault_count = s_history_count;
    memcpy(status->active_faults, s_active_faults, s_active_fault_count * sizeof(SystemMonitor_FaultRecord_t));
    
    xSemaphoreGive(s_system_monitor_mutex);
    return APP_OK;
}

// --- Private Helper Function Implementations ---

/**
 * @brief Adds a fault record to the historical log using circular buffer logic.
 * @param new_fault The fault record to add.
 */
static void SystemMonitor_AddHistoricalFault(const SystemMonitor_FaultRecord_t *new_fault) {
    s_historical_fault_log[s_history_write_idx] = *new_fault;
    s_history_write_idx = (s_history_write_idx + 1) % SysMon_HISTORY_SIZE;
    if (s_history_count < SysMon_HISTORY_SIZE) {
        s_history_count++;
    }
}

/**
 * @brief Adds a new fault or updates an existing one in the active fault list.
 * @param new_fault The fault record to add or update.
 */
static void SystemMonitor_AddActiveFault(const SystemMonitor_FaultRecord_t *new_fault) {
    // Check if the fault is already active
    for (uint32_t i = 0; i < s_active_fault_count; i++) {
        if (s_active_faults[i].id == new_fault->id) {
            // Check if the severity is higher, if so, update
            if (new_fault->severity > s_active_faults[i].severity) {
                s_active_faults[i].severity = new_fault->severity;
            }
            s_active_faults[i].timestamp_ms = new_fault->timestamp_ms; // Update timestamp
            s_active_faults[i].data = new_fault->data; // Update data
            return;
        }
    }

    // If not found, add it as a new active fault
    if (s_active_fault_count < SysMon_MAX_ACTIVE_FAULTS) {
        s_active_faults[s_active_fault_count] = *new_fault;
        s_active_fault_count++;
    } else {
        // Active fault log is full, overwrite the oldest entry (entry 0 in this case, by shifting)
        for (uint32_t i = 0; i < SysMon_MAX_ACTIVE_FAULTS - 1; i++) {
            s_active_faults[i] = s_active_faults[i + 1];
        }
        s_active_faults[SysMon_MAX_ACTIVE_FAULTS - 1] = *new_fault;
    }
}