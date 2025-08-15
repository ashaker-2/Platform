#ifndef STARTUP_H
#define STARTUP_H

#include "common.h"
#include <stdint.h>

/**
 * @file startup.h
 * @brief Public interface for the SystemStartup component.
 *
 * This header defines the main entry point for the application.
 */

// --- Public Functions ---

/**
 * @brief The main entry function for the application.
 *
 * This function performs essential initializations and starts the FreeRTOS scheduler.
 * This function is typically called by the underlying MCU SDK/bootloader after reset.
 */
// void main(void);

#endif // STARTUP_H