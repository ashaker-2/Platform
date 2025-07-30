// ecual/inc/ecual_timer_config.h

#ifndef ECUAL_TIMER_CONFIG_H
#define ECUAL_TIMER_CONFIG_H

#include <stdint.h>
#include "ecual_timer.h" // Includes ecual_timer.h for ECUAL_TIMER_Config_t and ECUAL_TIMER_Internal_Map_t definitions

/**
 * @brief User-friendly IDs for configured Timer instances.
 * Add an entry here for each physical Timer you configure.
 */
typedef enum ECUAL_TIMER_ID_t {
    ECUAL_TIMER_APP_UPDATE = 0, ///< For periodic application updates (e.g., 100ms)
    ECUAL_TIMER_HEARTBEAT,      ///< For system heartbeat (e.g., 1 second)
    ECUAL_TIMER_TOTAL_TIMERS    ///< Keep this last to get the count
} ECUAL_TIMER_ID_t;


// Declare the external constant arrays defined in ecual_timer_config.c
extern const ECUAL_TIMER_Config_t timer_configurations[];
extern const ECUAL_TIMER_Internal_Map_t timer_id_map[];

// Declare the external constant for the number of configurations
extern const uint32_t ECUAL_NUM_TIMER_CONFIGURATIONS;

#endif /* ECUAL_TIMER_CONFIG_H */
