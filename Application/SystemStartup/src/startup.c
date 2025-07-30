// main/startup.c

// ECUAL Includes (minimal for early logging)
#include "ecual_uart.h"

// Application Modules Includes (only logger for early logs)
#include "logger.h"

// Rte (Runtime Environment) Tasks Includes - now the central point for task creation and init tasks
#include "Rte.h"

// FreeRTOS Includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Standard C Library Includes
#include <stdio.h>

static const char *APP_TAG = "STARTUP";

/**
 * @brief Main application entry point.
 * This function handles the bare minimum system initialization to get FreeRTOS running
 * and then launches the Rte initialization function which handles all task creation.
 */
void app_main(void) {
    // --- 1. Basic ECUAL Initialization for Logging ---
    // Initialize ECUAL UART first, as the Logger depends on it.
    if (ECUAL_UART_Init() != ECUAL_OK) { 
        printf("CRITICAL ERROR: UART ECUAL Init failed! System cannot proceed.\r\n");
        while(1) {} // Halt if critical failure
    }
    
    // Initialize the generic Logger. From this point, use LOGx macros.
    if (LOGGER_Init() != APP_OK) { 
        printf("CRITICAL ERROR: Logger Init failed! Serial logging may not work reliably.\r\n"); 
    }
    
    LOGI(APP_TAG, "Minimal system setup complete. Calling RTE_Init to create initial tasks.");

    // --- Call RTE's master initialization function ---
    // RTE_Init will be responsible for creating the first init task (RTE_HwInitTask).
    if (RTE_Init() != APP_OK) {
        LOGE(APP_TAG, "RTE_Init failed! System cannot proceed.");
        while(1) {} // Halt if critical failure
    }

    // --- Start the FreeRTOS scheduler ---
    LOGI(APP_TAG, "Scheduler starting...");
    vTaskStartScheduler();

    // The code below this line will only be reached if there's an error in the scheduler
    LOGE(APP_TAG, "Scheduler exited unexpectedly! Entering infinite loop.");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}
