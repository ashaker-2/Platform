
/* ============================================================================
 * SOURCE FILE: Application/SystemMonitor/src/system_monitor.c
 * ============================================================================ */

#include "system_monitor.h"
#include "system_monitor_cfg.h"
#include "logger.h"
#include "common.h"
#include "Rte.h"

/* FreeRTOS includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <math.h>



typedef struct {
    uint32_t accumulated_cpu_load;
    uint32_t samples_count;
    uint32_t window_start_time;
    uint32_t hyperperiod_ms;
    uint32_t last_idle_run_time;
    bool hyperperiod_complete;
    uint8_t current_load_percent;
} core_cpu_context_t;

typedef struct {
    uint32_t accumulated_system_load;
    uint32_t samples_count;
    uint32_t window_start_time;
    bool system_hyperperiod_complete;
    uint8_t system_load_percent;
} system_cpu_context_t;



/* --- Internal Data Structures --- */

/* System health metrics */
static uint8_t u8ClearAllFaultRequest = 0;
static uint8_t s_current_cpu_load_percent = 0;
static uint32_t s_total_min_free_stack_bytes = 0;
static uint32_t s_active_task_count = 0;

/* Module state */
static bool s_is_initialized = false;
static SemaphoreHandle_t s_system_monitor_mutex = NULL;
static QueueHandle_t s_sysmon_queue = NULL;

static core_cpu_context_t s_core_contexts[configNUMBER_OF_CORES] = {
    {.hyperperiod_ms = CORE0_HYPERPERIOD_MS},
    {.hyperperiod_ms = CORE1_HYPERPERIOD_MS}
};

static system_cpu_context_t s_system_context = {0};


/* --- Private Function Prototypes --- */
static SystemMonitor_FaultRecord_t *sysmon_find_fault_record(SystemMonitor_FaultId_t fault_id);
static void sysmon_monitor_system_health(void);
static void sysmon_calculate_stack_usage(void);
static void sysmon_log_system_health(void);
static void sysmon_ClearFaults(void);
static void sysmon_init_cpu_load_monitoring(void);
static uint32_t gcd(uint32_t a, uint32_t b);
static uint32_t lcm(uint32_t a, uint32_t b);
static uint32_t calculate_hyperperiod(const uint32_t periods[], size_t count);
static void sysmon_calculate_cpu_load(void);
static void sysmon_print_stack_report(void);

/* --- Public Function Implementations --- */

Status_t SystemMonitor_Init(void)
{
    /* Create mutex for thread safety */
    s_system_monitor_mutex = xSemaphoreCreateMutex();
    if (s_system_monitor_mutex == NULL)
    {
        return E_ERROR;
    }

    s_sysmon_queue = xQueueCreate(SYSMON_QUEUE_LENGTH, sizeof(SystemMonitor_FaultRecord_t));
    if (s_sysmon_queue == NULL) {
        return E_NOK;
    }

    sysmon_init_cpu_load_monitoring();
    /* Reset system health metrics */
    s_current_cpu_load_percent = 0;
    s_total_min_free_stack_bytes = 0;
    s_active_task_count = 0;
    u8ClearAllFaultRequest = 0;

    s_is_initialized = true;

    return E_OK;
}

Status_t SysMon_ReportFaultStatus(SystemMonitor_FaultId_t fault_id, SysMon_FaultStatus_t status)
{
    if (!s_is_initialized)
        return E_ERROR;

    if ((fault_id <= FAULT_ID_NONE) || (fault_id >= FAULT_ID_MAX))
        return E_INVALID_PARAM;

    if ((status != FAULT_Active) && (status != FAULT_Paasive))
        return E_INVALID_PARAM;

    SystemMonitor_FaultRecord_t evt = { .u32FaultId = fault_id, .is_active = status };

    if (xQueueSend(s_sysmon_queue, &evt, SYSMON_QUEUE_WAIT_MS) != pdTRUE) 
    {
        // queue full → optional: drop or overwrite
        return E_BUSY;
    }

    return E_OK;
}

Status_t SysMon_ClearAllFaults(void)
{
    Status_t ret = E_OK;

    if (!s_is_initialized)
    {
        return E_ERROR;
    }

    u8ClearAllFaultRequest = 1;

    LOGI("SystemMonitor:"," Clear All fault Requested");

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
    SystemMonitor_FaultRecord_t evt;

    if (!s_is_initialized)
    {
        return;
    }

    if (xSemaphoreTake(s_system_monitor_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
    {
        return;
    }


    
    while (xQueueReceive(s_sysmon_queue, &evt, 0) == pdTRUE) 
    {
        SystemMonitor_FaultRecord_t *record = sysmon_find_fault_record(evt.u32FaultId);
        if (record) 
        {
            record->is_active = evt.is_active;
            LOGI("SysMon", "Fault updated: ID=0x%04X, status=%d", evt.u32FaultId, evt.is_active);
        }
        xSemaphoreGive(s_system_monitor_mutex);
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

uint32_t SysMon_GetTotalMinFreeStack(void)
{
    if (!s_is_initialized)
    {
        return 0;
    }

    /* Atomic read for 32-bit value on 32-bit systems */
    return s_total_min_free_stack_bytes;
}

Status_t SysMon_GetFaultStatus(SystemMonitor_FaultId_t FaultId, SysMon_FaultStatus_t *FaultStatus)
{
    Status_t ret = E_OK;
    if (!s_is_initialized || FaultStatus == NULL)
    {
        return E_ERROR;
    }

    if (FaultId == FAULT_ID_NONE || FaultId >= FAULT_ID_MAX)
    {
        LOGE("SystemMonitor:"," Invalid fault ID: 0x%04X", FaultId);
        return E_ERROR;
    }

    /* Search for fault active in table */
    SystemMonitor_FaultRecord_t *record = sysmon_find_fault_record(FaultId);

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

static SystemMonitor_FaultRecord_t *sysmon_find_fault_record(SystemMonitor_FaultId_t fault_id)
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

}


/**
 * @brief Calculate GCD using Euclidean algorithm
 */
static uint32_t gcd(uint32_t a, uint32_t b) 
{
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * @brief Calculate LCM of two numbers
 */
static uint32_t lcm(uint32_t a, uint32_t b) 
{
    return (a / gcd(a, b)) * b;
}

/**
 * @brief Calculate hyperperiod for given periods array
 */
static uint32_t calculate_hyperperiod(const uint32_t periods[], size_t count) 
{
    if (count == 0) return 0;
    
    uint32_t result = periods[0];
    for (size_t i = 1; i < count; i++) {
        result = lcm(result, periods[i]);
    }
    return result;
}

/**
 * @brief Initialize CPU load monitoring with actual task periods
 */
static void sysmon_init_cpu_load_monitoring(void) 
{
/* Core 0 task periods */
 const uint32_t core0_periods[] = {20, 100, 150, 200};
 const size_t core0_period_count = sizeof(core0_periods) / sizeof(core0_periods[0]);
 
/* Core 1 task periods */
 const uint32_t core1_periods[] = {50};
 const size_t core1_period_count = sizeof(core1_periods) / sizeof(core1_periods[0]);

/* System hyperperiod is LCM of all periods */
uint32_t all_periods[NUMBER_OF_ALL_PERIODS] = {20, 50, 100, 150, 200};

    /* Calculate actual hyperperiods */
    uint32_t calculated_core0_hp = calculate_hyperperiod(core0_periods, core0_period_count);
    uint32_t calculated_core1_hp = calculate_hyperperiod(core1_periods, core1_period_count);
    uint32_t calculated_system_hp = calculate_hyperperiod(all_periods, 5);
    
    LOGI("SystemMonitor:", "Calculated hyperperiods - Core0: %lums, Core1: %lums, System: %lums",
         calculated_core0_hp, calculated_core1_hp, calculated_system_hp);
    
    /* Update contexts with calculated values */
    s_core_contexts[0].hyperperiod_ms = calculated_core0_hp;
    s_core_contexts[1].hyperperiod_ms = calculated_core1_hp;
    
    /* Initialize timing */
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    for (int i = 0; i < configNUMBER_OF_CORES ; i++) {
        s_core_contexts[i].window_start_time = current_time;
    }
    s_system_context.window_start_time = current_time;
    
    LOGI("SystemMonitor:", "CPU load monitoring initialized with hyperperiod awareness");
}

/**
 * @brief Calculate per-core and system CPU load with hyperperiod awareness
 */
static void sysmon_calculate_cpu_load(void)
{
    static TickType_t last_measure_total_run_time = 0;
    static bool first_measurement = true;

    TaskStatus_t *task_status_array;
    UBaseType_t initial_task_count;
    uint32_t current_total_run_time;
    uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    /* Calculate per-core CPU loads */
    uint32_t total_system_idle_delta = 0;

    initial_task_count = uxTaskGetNumberOfTasks();
    s_active_task_count = initial_task_count;

    task_status_array = pvPortMalloc(initial_task_count * sizeof(TaskStatus_t));
    if (task_status_array == NULL) 
    {
        LOGE("SystemMonitor:", " Failed to allocate memory for task status array.");
        return;
    }

    UBaseType_t actual_task_count = uxTaskGetSystemState(task_status_array, 
                                                         initial_task_count, 
                                                         &current_total_run_time);

    if (!first_measurement && last_measure_total_run_time != 0) 
    {
        uint32_t core_idle_times[2] = {0, 0};
        uint32_t total_time_delta = current_total_run_time - last_measure_total_run_time;
        
        /* Collect idle times per core */
        for (UBaseType_t i = 0; i < actual_task_count; i++) 
        {
            const char* task_name = task_status_array[i].pcTaskName;
            
            if (strcmp(task_name, "IDLE0") == 0) 
            {
                core_idle_times[0] = task_status_array[i].ulRunTimeCounter;
            } 
            else if (strcmp(task_name, "IDLE1") == 0) 
            {
                core_idle_times[1] = task_status_array[i].ulRunTimeCounter;
            }
            else if (strcmp(task_name, "IDLE") == 0) 
            {
                /* Single core or legacy naming */
                core_idle_times[0] = task_status_array[i].ulRunTimeCounter;
            }
        }
        
        for (int core = 0; core < 2; core++) {
            if (total_time_delta > 0) {
                uint32_t core_idle_delta = core_idle_times[core] - s_core_contexts[core].last_idle_run_time;
                total_system_idle_delta += core_idle_delta;
                
                /* Per-core CPU usage calculation */
                if (core_idle_delta <= total_time_delta) {
                    uint32_t core_work_time = total_time_delta - core_idle_delta;
                    uint32_t core_cpu_usage = (core_work_time * 100) / total_time_delta;
                    
                    /* Accumulate for hyperperiod averaging */
                    s_core_contexts[core].accumulated_cpu_load += core_cpu_usage;
                    s_core_contexts[core].samples_count++;
                } else {
                    /* Handle overflow case */
                    s_core_contexts[core].accumulated_cpu_load += 0;
                    s_core_contexts[core].samples_count++;
                }
                
                s_core_contexts[core].last_idle_run_time = core_idle_times[core];
            }
            
            /* Check if core hyperperiod is complete */
            uint32_t core_window_duration = current_time_ms - s_core_contexts[core].window_start_time;
            
            if (core_window_duration >= s_core_contexts[core].hyperperiod_ms && 
                s_core_contexts[core].samples_count >= MIN_SAMPLES_PER_CORE) {
                
                /* Calculate average CPU load for this core over hyperperiod */
                uint32_t avg_core_load = s_core_contexts[core].accumulated_cpu_load / 
                                       s_core_contexts[core].samples_count;
                
                s_core_contexts[core].current_load_percent = (avg_core_load > 100) ? 100 : (uint8_t)avg_core_load;
                s_core_contexts[core].hyperperiod_complete = true;
                
                // LOGI("SystemMonitor:", "Core %d hyperperiod complete: %lu samples, avg load: %u%%, window: %lums",
                //      core, s_core_contexts[core].samples_count, avg_core_load, core_window_duration);
                
                /* Reset for next hyperperiod */
                s_core_contexts[core].accumulated_cpu_load = 0;
                s_core_contexts[core].samples_count = 0;
                s_core_contexts[core].window_start_time = current_time_ms;
            }
        }
        
        /* Calculate overall system CPU load */
        if (total_time_delta > 0) 
        {
            const uint32_t num_cores = configNUMBER_OF_CORES ? 1 : 2;
            uint32_t total_possible_work_time = total_time_delta * num_cores;
            
            if (total_system_idle_delta > total_possible_work_time) {
                total_system_idle_delta = total_possible_work_time;
            }
            
            uint32_t system_work_time = total_possible_work_time - total_system_idle_delta;
            uint32_t system_cpu_usage = (system_work_time * 100) / total_possible_work_time;
            
            s_system_context.accumulated_system_load += system_cpu_usage;
            s_system_context.samples_count++;
        }
        
        /* Check if system hyperperiod is complete */
        uint32_t system_window_duration = current_time_ms - s_system_context.window_start_time;
        
        if (system_window_duration >= SYSTEM_HYPERPERIOD_MS && 
            s_system_context.samples_count >= MIN_SAMPLES_PER_CORE) {
            
            uint32_t avg_system_load = s_system_context.accumulated_system_load / 
                                     s_system_context.samples_count;
            
            s_system_context.system_load_percent = (avg_system_load > 100) ? 100 : (uint8_t)avg_system_load;
            s_system_context.system_hyperperiod_complete = true;
            
            /* Also update global for compatibility */
            s_current_cpu_load_percent = s_system_context.system_load_percent;
            
            // LOGI("SystemMonitor:", "System hyperperiod complete: %lu samples, avg load: %u%%, window: %lums",
            //      s_system_context.samples_count, avg_system_load, system_window_duration);
            
            /* Reset for next system hyperperiod */
            s_system_context.accumulated_system_load = 0;
            s_system_context.samples_count = 0;
            s_system_context.window_start_time = current_time_ms;
        }
    } else {
        /* First measurement - initialize baseline values */
        for (UBaseType_t i = 0; i < actual_task_count; i++) {
            const char* task_name = task_status_array[i].pcTaskName;
            
            if (strcmp(task_name, "IDLE0") == 0) {
                s_core_contexts[0].last_idle_run_time = task_status_array[i].ulRunTimeCounter;
            } else if (strcmp(task_name, "IDLE1") == 0) {
                s_core_contexts[1].last_idle_run_time = task_status_array[i].ulRunTimeCounter;
            } else if (strcmp(task_name, "IDLE") == 0) {
                s_core_contexts[0].last_idle_run_time = task_status_array[i].ulRunTimeCounter;
            }
        }
        first_measurement = false;
    }

    last_measure_total_run_time = current_total_run_time;
    vPortFree(task_status_array);
}

/**
 * @brief Get CPU load for specific core
 */
uint8_t sysmon_get_core_cpu_load(uint8_t core_id, bool *is_hyperperiod_complete) 
{
    if (core_id >= 2) return 0;
    
    if (is_hyperperiod_complete) {
        *is_hyperperiod_complete = s_core_contexts[core_id].hyperperiod_complete;
    }
    
    return s_core_contexts[core_id].current_load_percent;
}

/**
 * @brief Get overall system CPU load
 */
uint8_t sysmon_get_system_cpu_load(bool *is_hyperperiod_complete) 
{
    if (is_hyperperiod_complete) {
        *is_hyperperiod_complete = s_system_context.system_hyperperiod_complete;
    }
    
    return s_system_context.system_load_percent;
}

/**
 * @brief Print comprehensive CPU load report
 */
void sysmon_print_cpu_load_report(void) 
{
    bool core0_valid, core1_valid, system_valid;
    
    uint8_t core0_load = sysmon_get_core_cpu_load(0, &core0_valid);
    uint8_t core1_load = sysmon_get_core_cpu_load(1, &core1_valid);
    uint8_t system_load = sysmon_get_system_cpu_load(&system_valid);
    
    if(system_load >= SYSMON_CPU_LOAD_THRESHOLD_PERCENT && system_load == 1)
    {
        SysMon_ReportFaultStatus(FAULT_ID_OVERALL_CPU_LOAD, true);
    }
    else
    {
        SysMon_ReportFaultStatus(FAULT_ID_OVERALL_CPU_LOAD, false);
    }
    
    if(core0_load >= SYSMON_CPU_LOAD_THRESHOLD_PERCENT && core0_valid == 1)
    {
        SysMon_ReportFaultStatus(FAULT_ID_CORE_0_CPU_LOAD, true);
    }
    else
    {
        SysMon_ReportFaultStatus(FAULT_ID_CORE_0_CPU_LOAD, false);
    }

    if(core1_load >= SYSMON_CPU_LOAD_THRESHOLD_PERCENT && core1_valid == 1)
    {
        SysMon_ReportFaultStatus(FAULT_ID_CORE_1_CPU_LOAD, true);
    }
    else
    {
        SysMon_ReportFaultStatus(FAULT_ID_CORE_1_CPU_LOAD, false);
    }
    

    LOGI("SystemMonitor:", "=== CPU Load Report ===");
    LOGI("SystemMonitor:", "Core 0: %u%% %s(HP: %lums)", 
         core0_load, core0_valid ? "[VALID] " : "[PENDING] ", CORE0_HYPERPERIOD_MS);
    LOGI("SystemMonitor:", "Core 1: %u%% %s(HP: %lums)", 
         core1_load, core1_valid ? "[VALID] " : "[PENDING] ", CORE1_HYPERPERIOD_MS);
    LOGI("SystemMonitor:", "System: %u%% %s(HP: %lums)", 
         system_load, system_valid ? "[VALID] " : "[PENDING] ", SYSTEM_HYPERPERIOD_MS);
    LOGI("SystemMonitor:", "======================");
}


/**
 * @brief Print comprehensive CPU load report
 */
static void sysmon_print_stack_report(void) 
{
    if(s_total_min_free_stack_bytes <= SYSMON_MIN_FREE_STACK_THRESHOLD_BYTES)
    {
        SysMon_ReportFaultStatus(FAULT_ID_STACK_OVERFLOW, true);
    }
    else
    {
        SysMon_ReportFaultStatus(FAULT_ID_STACK_OVERFLOW, false);
    }

    if(s_total_min_free_stack_bytes <= 256)
    {
        SysMon_ReportFaultStatus(FAULT_ID_STACK_UNDERFLOW, true);
    }
    else
    {
        SysMon_ReportFaultStatus(FAULT_ID_STACK_UNDERFLOW, false);
    }

    LOGI("SystemMonitor:", "=== Stack Report ===");
    LOGI("SystemMonitor:", "Current free Stack : %d bytes", s_total_min_free_stack_bytes);
    LOGI("SystemMonitor:", "======================");
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
        LOGE("SystemMonitor:"," Failed to allocate memory for task status array (stack usage calculation).");
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
    vPortFree(task_status_array);                                // Free allocated memory
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
    if ((log_cycle_counter % cycles_per_log) == 0)
    {
        sysmon_print_cpu_load_report();
        sysmon_print_stack_report();

    }
}