#include "startup.h"
// #include "logger.h"
#include "Rte.h"
#include "system_monitor.h"
#include "hal_uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @file startup.c
 * @brief Implementation for the SystemStartup component.
 *
 * This file contains the main entry point for the firmware, responsible for
 * performing initializations and starting the RTOS scheduler.
 */

// --- Public Function Implementations ---

/**
 * @brief The main entry function for the application.
 *
 * @details This function is the first user-code to run after boot. It
 * initializes critical services (like logging), triggers the RTE initialization,
 * and starts the FreeRTOS scheduler to begin multi-tasking.
 */
void app_main(void)
{

    // 1. Minimal early hardware setup for debug logging
    // This assumes the HAL_UART_Init function is safe to call before the full
    // system is up.
    // HAL_UART_Init(UART_DEBUG_PORT, BAUD_RATE_115200);

    // 2. Initialize the logger to enable early debug output
    // if (LOGGER_Init() != E_OK)
    // {
    //     // Critical failure: The logger is essential. Halt execution.
    //     // We cannot report this fault as the logger is not working.
    //     while (1)
    //     {
    //         // Blink an LED or other hardware-specific indication of a fatal error
    //     }
    // }
    // LOGI("SystemStartup", "Logger initialized. Application starting...");

    // 3. Initialize SystemMonitor before other components to enable fault reporting
    // if (SysMon_Init() != E_OK)
    // {
        // LOGF("SystemStartup", "FATAL: SystemMonitor initialization failed, halting system.");
        // while (1)
        // {
        // }
    // }
    // LOGI("SystemStartup", "SystemMonitor initialized.");

    // 4. Initialize the RTE
    // This call is expected to create the initial hardware initialization task
    // and call Init() functions of all other components.
    // if (RTE_Init() != E_OK)
    // {
        // LOGF("SystemStartup", "FATAL: RTE initialization failed, halting system.");
        // Report the fault, but then halt.
        // SysMon_ReportFault(FAULT_ID_SYS_INIT_ERROR, SEVERITY_CRITICAL, 0);
        // while (1)
        // {
        // }
    // }
    // LOGI("SystemStartup", "RTE initialized. All components initialized.");

    // // 5. Start the FreeRTOS scheduler
    // LOGI("SystemStartup", "Starting FreeRTOS scheduler...");
    // vTaskStartScheduler();

    // // The scheduler takes control from this point.
    // // The code below should not be reached unless the scheduler fails.
    // // LOGF("SystemStartup", "FATAL: Scheduler failed to start!");
    // // SysMon_ReportFault(FAULT_ID_SYS_INIT_ERROR, SEVERITY_CRITICAL, 1);
    while (1)
    {
    }

}