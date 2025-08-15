#ifndef SDKCONFIG_H
#define SDKCONFIG_H

/* ----- FreeRTOS related ----- */
#define CONFIG_FREERTOS_UNICORE                         0
#define CONFIG_FREERTOS_SMP                             1
#define CONFIG_FREERTOS_HZ                              1000
#define CONFIG_FREERTOS_MAX_PRIORITIES                  25
#define CONFIG_FREERTOS_IDLE_TASK_STACKSIZE             768
#define CONFIG_FREERTOS_ISR_STACKSIZE                   2048
#define CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES 1
#define CONFIG_FREERTOS_TASK_FUNCTION_WRAPPER           1
#define CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS   5
#define CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY      1
#define CONFIG_FREERTOS_CHECK_MUTEX_GIVEN_BY_OWNER      1
#define CONFIG_FREERTOS_USE_TRACE_FACILITY              1
#define CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS         0
#define CONFIG_FREERTOS_SUPPORT_STATIC_ALLOCATION       1
#define CONFIG_FREERTOS_SUPPORT_DYNAMIC_ALLOCATION      1
#define CONFIG_FREERTOS_USE_TICKLESS_IDLE               0
#define CONFIG_FREERTOS_TICK_SUPPORT_CORE_AFFINITY      1

/* ----- CPU / SOC ----- */
#define CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ               240
#define CONFIG_ESP32_XTAL_FREQ                          40
#define CONFIG_ESP32_XTAL_FREQ_40                       1
#define CONFIG_ESP_MAIN_TASK_STACK_SIZE                 4096
#define CONFIG_LOG_VERSION                              2
#define CONFIG_ESP_MAIN_TASK_AFFINITY                   0




/* ----- Heap and memory ----- */
#define CONFIG_ESP32_SPIRAM_SUPPORT                     0
#define CONFIG_ESP32_DEFAULT_PSRAM_SIZE                 -1

/* ----- Misc ----- */
#define CONFIG_LOG_DEFAULT_LEVEL                        3
#define CONFIG_LOG_MAXIMUM_LEVEL                        5


/* ----- ESP-IDF Target ----- */
#define CONFIG_IDF_TARGET_ESP32                          1
#define CONFIG_IDF_TARGET                                "esp32"

/* ----- Core Mode ----- */
// 0 = dual core (SMP enabled), 1 = single core mode
#define CONFIG_ESP_SYSTEM_SINGLE_CORE_MODE              0

/* ----- IRAM 8-bit accessible memory ----- */
#define CONFIG_ESP32_IRAM_AS_8BIT_ACCESSIBLE_MEMORY     1

#endif /* SDKCONFIG_H */
