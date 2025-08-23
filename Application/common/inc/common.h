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
    E_ERROR = -1,              // General unspecified error. Use this when a more specific error isn't available.
    E_OK = 0,             // Operation completed successfully.
    E_NOK = 1,             // Operation completed successfully.
    E_INVALID_PARAM,      // A function received an invalid or out-of-range parameter.
    E_NOT_INITIALIZED,    // A module or component was used before being properly initialized.
    E_TIMEOUT,            // An operation failed due to a timeout (e.g., waiting for a response, mutex).
    E_BUSY,               // A resource is currently busy and cannot fulfill the request at this time.
    E_MUTEX_ERROR,        // An error occurred during a mutex (semaphore) operation (e.g., acquisition failure).
    E_HAL_INVALID_PARAM,
    E_NULL_ERROR,
    E_INVALID_ARG,          /**< Invalid argument provided */
    E_NULL_POINTER,         /**< Null pointer encountered */
    E_NO_RESOURCE,          /**< No resource available */
    E_ALREADY_INITIALIZED,  /**< Module is already initialized */
    E_NOT_SUPPORTED,        /**< Feature/operation not supported */
    E_COMM_ERROR,           /**< Communication error with sensor */
    E_SENSOR_ERROR,         /**< Sensor internal error */
    E_DATA_STALE,           /**< Sensor data is stale/not yet updated, or last read failed */
    E_NOT_CONFIGURED,        /**< Sensor is not configured */
    E_FAIL,
    E_NOT_FOUND
} Status_t;

/**
 * @brief Provides a generic way to get system uptime in milliseconds.
 * This function should be implemented in the platform-specific ECUAL common source.
 * @return System uptime in milliseconds.
 */
uint64_t APP_GetUptimeMs(void);

#endif /* COMMON_H */

