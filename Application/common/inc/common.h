// common.h

#ifndef COMMON_H
#define COMMON_H


#include <stdint.h>

// ... existing declarations ...
#define true	1
#define false	0
// Define common status codes that can be used across various modules.
// This enumeration provides a standardized way for functions to report
// their outcome (success or specific types of failure).
typedef enum {
    E_OK = 0,             // Operation completed successfully.
    E_NOK = 1,             // Operation completed successfully.
    E_ERROR,              // General unspecified error. Use this when a more specific error isn't available.
    E_INVALID_PARAM,      // A function received an invalid or out-of-range parameter.
    E_NOT_INITIALIZED,    // A module or component was used before being properly initialized.
    E_TIMEOUT,            // An operation failed due to a timeout (e.g., waiting for a response, mutex).
    E_BUSY,               // A resource is currently busy and cannot fulfill the request at this time.
    E_MUTEX_ERROR,        // An error occurred during a mutex (semaphore) operation (e.g., acquisition failure).
    E_HAL_INVALID_PARAM,
    E_NULL_ERROR,
    // Add more common status codes as needed for your system, for example:
    // COMMON_NO_MEMORY,       // Failed to allocate memory.
    // COMMON_NOT_SUPPORTED,   // Feature or operation is not supported.
} Status_t;

/**
 * @brief Provides a generic way to get system uptime in milliseconds.
 * This function should be implemented in the platform-specific ECUAL common source.
 * @return System uptime in milliseconds.
 */
uint64_t APP_GetUptimeMs(void);

#endif /* COMMON_H */

