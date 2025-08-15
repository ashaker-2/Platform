#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
#include "sdkconfig.h"

/* SMP (Dual core) settings */
#define configNUMBER_OF_CORES                    2
#define portNUM_PROCESSORS                       2
#define configUSE_CORE_AFFINITY                  1
#define configRUN_MULTIPLE_PRIORITIES            1



/* Basic kernel settings */
#define configUSE_PREEMPTION                     1
#define configUSE_IDLE_HOOK                      0
#define configUSE_PASSIVE_IDLE_HOOK              0
#define configUSE_TICK_HOOK                      0
#define configTICK_RATE_HZ                       ((TickType_t)CONFIG_FREERTOS_HZ)
#define configMAX_PRIORITIES                     (CONFIG_FREERTOS_MAX_PRIORITIES)
#define configMINIMAL_STACK_SIZE                 (CONFIG_FREERTOS_IDLE_TASK_STACKSIZE)
#define configTOTAL_HEAP_SIZE                    (64 * 1024)
#define configMAX_TASK_NAME_LEN                  16
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  1

#define CONFIG_FREERTOS_MAX_TASK_NAME_LEN        16
#define configISR_STACK_SIZE                     4096
#define configRECORD_STACK_HIGH_ADDRESS          1
#define configSUPPORT_STATIC_ALLOCATION          1
// #define configSUPPORT_DYNAMIC_ALLOCATION         1



/* Task notifications */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES    CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES

/* Mutexes, semaphores, etc. */
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES             1
#define configQUEUE_REGISTRY_SIZE                 10

/* Software timers */
#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                 (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                  10
#define configTIMER_TASK_STACK_DEPTH              (2048)

/* Memory allocation */
#define configSUPPORT_STATIC_ALLOCATION           CONFIG_FREERTOS_SUPPORT_STATIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION          CONFIG_FREERTOS_SUPPORT_DYNAMIC_ALLOCATION

/* Debug / runtime stats */
#define configGENERATE_RUN_TIME_STATS             CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#define configUSE_TRACE_FACILITY                  CONFIG_FREERTOS_USE_TRACE_FACILITY
#define configUSE_STATS_FORMATTING_FUNCTIONS      1
#define configCHECK_FOR_STACK_OVERFLOW            2
#define configUSE_MALLOC_FAILED_HOOK              1

/* Hook functions */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS   CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS

/* Interrupt priorities */
#define configKERNEL_INTERRUPT_PRIORITY           1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY      5

/* Assert macro */
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }



/* -------------------- Trace Macros ----------------------- */
#ifndef __ASSEMBLER__
#if CONFIG_SYSVIEW_ENABLE
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#undef INLINE /* to avoid redefinition */
#endif /* CONFIG_SYSVIEW_ENABLE */
#if CONFIG_FREERTOS_SMP
/* Default values for trace macros added to ESP-IDF implementation of SYSVIEW
 * that is not part of Amazon SMP FreeRTOS. */
#ifndef traceISR_EXIT
#define traceISR_EXIT()
#endif
#ifndef traceISR_ENTER
#define traceISR_ENTER(_n_)
#endif
#ifndef traceQUEUE_GIVE_FROM_ISR
#define traceQUEUE_GIVE_FROM_ISR(pxQueue)
#endif
#ifndef traceQUEUE_GIVE_FROM_ISR_FAILED
#define traceQUEUE_GIVE_FROM_ISR_FAILED(pxQueue)
#endif
#ifndef traceQUEUE_SEMAPHORE_RECEIVE
#define traceQUEUE_SEMAPHORE_RECEIVE(pxQueue)
#endif
#endif /* CONFIG_FREERTOS_SMP */
#endif /* def __ASSEMBLER__ */

#if CONFIG_FREERTOS_USE_APPLICATION_TASK_TAG
#define configUSE_APPLICATION_TASK_TAG 1
#endif // CONFIG_FREERTOS_USE_APPLICATION_TASK_TAG


/* API function inclusion */
#define INCLUDE_vTaskPrioritySet                  1
#define INCLUDE_uxTaskPriorityGet                 1
#define INCLUDE_vTaskDelete                       1
#define INCLUDE_vTaskSuspend                      1
#define INCLUDE_vTaskDelayUntil                   1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_xTaskGetSchedulerState            1
#define INCLUDE_xTaskGetCurrentTaskHandle         1
#define INCLUDE_uxTaskGetStackHighWaterMark       1
#define INCLUDE_xTaskGetIdleTaskHandle            1
#define INCLUDE_eTaskGetState                     1
#define INCLUDE_xTimerPendFunctionCall            1
#define INCLUDE_xTaskAbortDelay                   1























#endif /* FREERTOS_CONFIG_H */
