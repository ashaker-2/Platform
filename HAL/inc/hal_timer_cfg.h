/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_Timer_Cfg.h
 * ============================================================================*/
/**
 * @file HAL_Timer_Cfg.h
 * @brief Declarations for external Timer configuration data.
 * This header makes the static Timer configuration array available to HAL_Timer.c
 * for initialization. It does not declare any functions.
 */
#ifndef HAL_TIMER_CFG_H
#define HAL_TIMER_CFG_H

#include "driver/timer.h"    // For timer_group_t, timer_idx_t, timer_config_t
#include <stddef.h>          // For size_t

/**
 * @brief Structure to hold timer configuration along with its group and index.
 * Defined here for external visibility, used in HAL_Timer_Cfg.c.
 */
typedef struct {
    timer_group_t group_id;
    timer_idx_t timer_id;
    timer_config_t config;
    uint64_t initial_alarm_value; // In microseconds, for setting alarm value
} timer_cfg_item_t;

/**
 * @brief External declaration of the array containing all predefined hardware timer configurations.
 * This array is defined in HAL_Timer_Cfg.c and accessed by HAL_Timer.c to perform
 * initial Timer setup.
 */
extern const timer_cfg_item_t s_timer_configurations[];

/**
 * @brief External declaration of the number of elements in the Timer configurations array.
 */
extern const size_t s_num_timer_configurations;

#endif /* HAL_TIMER_CFG_H */
