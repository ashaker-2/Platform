// ecual/inc/app_common.h

#ifndef APP_COMMON_H
#define APP_COMMON_H

#include <stdint.h>
#include "ecual_types.h" // Assuming this defines ECUAL_OK, ECUAL_ERROR

// ... existing declarations ...

/**
 * @brief Provides a generic way to get system uptime in milliseconds.
 * This function should be implemented in the platform-specific ECUAL common source.
 * @return System uptime in milliseconds.
 */
uint64_t APP_GetUptimeMs(void);

#endif /* APP_COMMON_H */
