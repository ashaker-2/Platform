
/* ============================================================================
 * SOURCE FILE: Application/SystemMonitor/src/system_monitor.c
 * ============================================================================ */

#include "system_monitor.h"
#include "system_monitor_cfg.h"
#include "logger.h"
#include "common.h"
#include "Rte.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

/* --- Internal Data Structures --- */

/* System health metrics */
static uint8_t u8ClearAllFaultRequest = 0;
static uint8_t s_current_cpu_load_percent = 0;
static uint32_t s_total_min_free_stack_bytes = 0;
static uint32_t s_active_task_count = 0;

/* Module state */
static bool s_is_initialized = false;
static SemaphoreHandle_t s_system_monitor_mutex = NULL;


/* --- Private Function Prototypes --- */
static SystemMonitor_FaultRecord_t* sysmon_find_fault_record(SystemMonitor_FaultId_t fault_id);
static void sysmon_monitor_system_health(void);
static void sysmon_calculate_cpu_load(void);
static void sysmon_calculate_stack_usage(void);
static void sysmon_log_system_health(void);
static void sysmon_ClearFaults(void);

/* --- Public Function Implementations --- */

Status_t SystemMonitor_Init(void)
{
    /* Create mutex for thread safety */
    s_system_monitor_mutex = xSemaphoreCreateMutex();
    if (s_system_monitor_mutex == NULL) 
    {
        return E_ERROR;
    }

    /* Reset system health metrics */
    s_current_cpu_load_percent = 0;
    s_total_min_free_stack_bytes = 0;
    s_active_task_count = 0;
    u8ClearAllFaultRequest = 0;


    s_is_initialized = true;


    return E_OK;
}

Status_t SysMon_ReportFaultStatus(SystemMonitor_FaultId_t fault_id,SysMon_FaultStatus_t status)
{
    Status_t ret = E_OK;
    /* Validate inputs */
    if (!s_is_initialized) 
    {
        return E_ERROR;
    }
    
    if (fault_id == FAULT_ID_NONE || fault_id >= FAULT_ID_MAX) 
    {
        LOGE("SystemMonitor: Invalid fault ID: 0x%04X", fault_id);
        return E_ERROR;
    }

    /* Acquire mutex with timeout to prevent blocking */
    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(10)) != pdTRUE) 
    {
        LOGE("SystemMonitor: Mutex timeout in ReportFault");
        return E_ERROR;
    }

    /* Set fault as active in table */

    SystemMonitor_FaultRecord_t* record = sysmon_find_fault_record(fault_id);
    
    if ((record != NULL) && ((status == FAULT_Paasive) || (status == FAULT_Active))) 
    {
        record->is_active = status;
        ret = E_OK;
    }
    else
    {
        ret = E_INVALID_PARAM;
    }


    /* Log the fault */
    if (ret == E_OK) 
    {
        LOGE("FAULT REPORTED: ID=0x%04X", fault_id);
    } 
    else 
    {
        LOGE("SystemMonitor: Failed to set fault 0x%04X", fault_id);
    }

    /* Release mutex */
    xSemaphoreGive(s_system_monitor_mutex);

    return ret;
}

Status_t SysMon_ClearAllFaults(void)
{
    Status_t ret = E_OK;

    if (!s_is_initialized) 
    {
        return E_ERROR;
    }

    u8ClearAllFaultRequest = 1;

    LOGI("SystemMonitor: Clear All fault Requested");
    
    return ret;
}

static void sysmon_ClearFaults(void)
{
    /* Initialize fault table - all faults start as inactive */
    for (uint8_t i = 0; i < SYSMON_MAX_FAULTS; i++) 
    {
        SystemMonitor_FaultTable[i].is_active = false;
    }
    u8ClearAllFaultRequest = 0;
}

void SysMon_MainFunction(void)
{
    if (!s_is_initialized) 
    {
        return;
    }

    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE) 
    {
        return;
    }

    if (u8ClearAllFaultRequest == 1) 
    {
        sysmon_ClearFaults();
    }

    /* Monitor system health (CPU, Stack) */
    sysmon_monitor_system_health();

    /* Periodic health logging */
    sysmon_log_system_health();

    xSemaphoreGive(s_system_monitor_mutex);
}

uint8_t SysMon_GetCPULoad(void)
{
    if (!s_is_initialized) 
    {
        return 0;
    }
    
    /* CPU load is atomic read, no mutex needed for single byte */
    return s_current_cpu_load_percent;
}

uint32_t SysMon_GetTotalMinFreeStack(void)
{
    if (!s_is_initialized) {
        return 0;
    }
    
    /* Atomic read for 32-bit value on 32-bit systems */
    return s_total_min_free_stack_bytes;
}

Status_t SysMon_GetFaultStatus(SystemMonitor_FaultId_t FaultId , SysMon_FaultStatus_t * FaultStatus)
{
    Status_t ret = E_OK;
    if (!s_is_initialized || FaultStatus == NULL) 
    {
        return E_ERROR;
    }

    if (FaultId == FAULT_ID_NONE || FaultId >= FAULT_ID_MAX) 
    {
        LOGE("SystemMonitor: Invalid fault ID: 0x%04X", FaultId);
        return E_ERROR;
    }

    /* Search for fault active in table */
    SystemMonitor_FaultRecord_t* record = sysmon_find_fault_record(FaultId);
    
    if (record != NULL) 
    {
        *FaultStatus = (SysMon_FaultStatus_t)record->is_active; 
        ret = E_OK;  
    }
    else
    {
        ret = E_NULL_ERROR;
    }
    return ret;
}


static SystemMonitor_FaultRecord_t* sysmon_find_fault_record(SystemMonitor_FaultId_t fault_id)
{
    for (uint8_t i = 0; i < SYSMON_MAX_FAULTS; i++) 
    {
        if (SystemMonitor_FaultTable[i].u32FaultId == fault_id) 
        {
            return &SystemMonitor_FaultTable[i];
        }
    }
    return NULL;
}


/**
 * @brief Monitors core system health metrics like CPU load and stack usage.
 *
 * This function calls `sysmon_calculate_cpu_load` and `sysmon_calculate_stack_usage`
 * to update internal health metrics. It then uses `SysMon_SetFaultStatus` to report
 * or clear the `FAULT_ID_CPU_OVERLOAD` and `FAULT_ID_STACK_OVERFLOW_RISK` faults
 * based on their configured thresholds.
 * It's assumed to be called within a context where the `s_system_monitor_mutex` is held.
 */
static void sysmon_monitor_system_health(void) 
{
    /* Update CPU load and stack usage metrics. */
    sysmon_calculate_cpu_load();
    sysmon_calculate_stack_usage();

    /* Check and set/clear CPU overload fault. */
    bool cpu_overload_active = (s_current_cpu_load_percent > SYSMON_CPU_LOAD_THRESHOLD_PERCENT);
    /* Check and set/clear stack overflow risk fault. */
    bool stack_risk_active = (s_total_min_free_stack_bytes < SYSMON_MIN_FREE_STACK_THRESHOLD_BYTES);

    SysMon_ReportFaultStatus(FAULT_ID_CPU_OVERLOAD, cpu_overload_active);
    SysMon_ReportFaultStatus(FAULT_ID_STACK_OVERFLOW_RISK, stack_risk_active);

    if (cpu_overload_active) 
    {
        LOGW("SystemMonitor: CPU overload: %d%% > %d%%",s_current_cpu_load_percent, SYSMON_CPU_LOAD_THRESHOLD_PERCENT);
    } 
    else 
    {

    }

    if (stack_risk_active) 
    {
        LOGW("SystemMonitor: CPU overload: %d%% > %d%%",s_current_cpu_load_percent, SYSMON_CPU_LOAD_THRESHOLD_PERCENT);
    } 
    else 
    {

    }
}

/**
 * @brief Calculates the current CPU load percentage.
 *
 * This function calculates CPU usage by observing the idle task's run-time counter.
 * It uses `uxTaskGetSystemState` and requires `configGENERATE_RUN_TIME_STATS` to be
 * enabled in FreeRTOS. It updates `s_current_cpu_load_percent` and `s_active_task_count`.
 * It's assumed to be called within a context where the `s_system_monitor_mutex` is held.
 */
static void sysmon_calculate_cpu_load(void) 
{
    static TickType_t last_measure_total_run_time = 0; // Stores total run time at last measurement
    static uint32_t last_idle_run_time = 0;            // Stores idle task run time at last measurement

    TaskStatus_t *task_status_array;
    UBaseType_t initial_task_count;
    uint32_t current_total_run_time;

    /* Get the current number of tasks for memory allocation. */
    initial_task_count = uxTaskGetNumberOfTasks();
    s_active_task_count = initial_task_count; // Update active task count

    /* Allocate memory for task status array. pvPortMalloc is FreeRTOS heap function. */
    task_status_array = pvPortMalloc(initial_task_count * sizeof(TaskStatus_t));
    if (task_status_array == NULL) 
    {
        LOGE("SystemMonitor: Failed to allocate memory for task status array (CPU load calculation).");
        return; // Exit if memory allocation fails.
    }

    /* Get current system state, including task run times. */
    // uxTaskGetSystemState returns the actual number of tasks copied.
    UBaseType_t actual_task_count = uxTaskGetSystemState(task_status_array, initial_task_count, &current_total_run_time);

    if (last_measure_total_run_time != 0) 
    { // Only calculate after the first measurement cycle
        uint32_t current_idle_run_time = 0;
        /* Find the "IDLE" task's run time. */
        for (UBaseType_t i = 0; i < actual_task_count; i++) 
        {
            if (strcmp(task_status_array[i].pcTaskName, "IDLE") == 0) 
            {
                current_idle_run_time = task_status_array[i].ulRunTimeCounter;
                break;
            }
        }

        /* Calculate delta for total run time and idle time. */
        uint32_t total_time_delta = current_total_run_time - last_measure_total_run_time;
        uint32_t idle_time_delta = current_idle_run_time - last_idle_run_time;

        if (total_time_delta > 0) 
        {
            /* CPU usage = 100 - (Idle time_delta / Total time_delta) * 100 */
            uint32_t cpu_usage = (uint32_t)(100.0f - ((float)idle_time_delta * 100.0f) / total_time_delta);
            s_current_cpu_load_percent = (cpu_usage > 100) ? 100 : (uint8_t)cpu_usage; // Cap at 100%
        } else 
        {
            s_current_cpu_load_percent = 0; // No time elapsed, assume 0% load.
        }

        last_idle_run_time = current_idle_run_time; // Store for next calculation
    }

    last_measure_total_run_time = current_total_run_time; // Store for next calculation
    vPortFree(task_status_array); // Free allocated memory
}

/**
 * @brief Calculates the total minimum free stack space across all tasks.
 *
 * This function sums the "high water mark" (minimum ever free stack) for each
 * active FreeRTOS task. This provides an indicator of how close tasks are
 * to overflowing their stacks. It updates `s_total_min_free_stack_bytes`.
 * It's assumed to be called within a context where the `s_system_monitor_mutex` is held.
 */
static void sysmon_calculate_stack_usage(void) 
{
    TaskStatus_t *task_status_array;
    UBaseType_t initial_task_count;
    uint32_t total_run_time_dummy; // Not used, but required by uxTaskGetSystemState
    uint32_t current_total_min_free_stack = 0;

    /* Get the current number of tasks for memory allocation. */
    initial_task_count = uxTaskGetNumberOfTasks();

    /* Allocate memory for task status array. */
    task_status_array = pvPortMalloc(initial_task_count * sizeof(TaskStatus_t));
    if (task_status_array == NULL) 
    {
        LOGE("SystemMonitor: Failed to allocate memory for task status array (stack usage calculation).");
        return;
    }

    /* Get current system state, including stack high water marks. */
    UBaseType_t actual_task_count = uxTaskGetSystemState(task_status_array, initial_task_count, &total_run_time_dummy);

    /* Sum the minimum free stack (high water mark) for all tasks.
     * `usStackHighWaterMark` is in words, convert to bytes using `sizeof(StackType_t)`. */
    for (UBaseType_t i = 0; i < actual_task_count; i++) 
    {
        current_total_min_free_stack += task_status_array[i].usStackHighWaterMark * sizeof(StackType_t);
    }

    s_total_min_free_stack_bytes = current_total_min_free_stack; // Update static variable
    vPortFree(task_status_array); // Free allocated memory
}

/**
 * @brief Periodically logs key system health metrics to the debug console.
 *
 * This function calculates whether it's time to log based on `SYSMON_HEALTH_LOG_INTERVAL_SEC`
 * and `SYSMON_POLLING_INTERVAL_MS`. It prints current CPU load, minimum free stack,
 * active task count, and system uptime.
 * It's assumed to be called within a context where the `s_system_monitor_mutex` is held.
 */
static void sysmon_log_system_health(void) 
{
    static uint32_t log_cycle_counter = 0;
    log_cycle_counter++;

    /* Calculate how many `SysMon_MainFunction` calls equate to the desired log interval. */
    uint32_t cycles_per_log = SYSMON_HEALTH_LOG_INTERVAL_SEC * 1000 / SYSMON_POLLING_INTERVAL_MS;
    if (cycles_per_log == 0)
    {
        cycles_per_log = 1; // Ensure logging happens at least every cycle if interval is tiny
    } 

    /* Log only when the calculated cycle count is reached. */
    if ((log_cycle_counter % cycles_per_log) == 0) {
        LOGI("SysHealth: CPU:%u%%, MinFreeStack:%luB, Tasks:%lu, Uptime:%lu ms",
             s_current_cpu_load_percent,
             s_total_min_free_stack_bytes,
             s_active_task_count,
             APP_COMMON_GetUptimeMs());
    }
}