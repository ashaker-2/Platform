/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_Timer.h
 * ============================================================================*/
/**
 * @file HAL_Timer.h
 * @brief Public API for interacting with hardware timers.
 * This header declares functions for initializing the timers,
 * starting/stopping them, and attaching interrupt callbacks.
 */
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "common/status.h"  // For Status_t
#include "driver/timer.h"   // For timer_group_t, timer_idx_t

/**
 * @brief Initializes and configures all hardware timers according to configurations
 * defined in `HAL_Timer_Cfg.c`. This is the main initialization function for the Timer HAL.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Init(void);

/**
 * @brief Starts a previously initialized timer.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer to start.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Start(timer_group_t group_id, timer_idx_t timer_id);

/**
 * @brief Stops a running timer.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer to stop.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Stop(timer_group_t group_id, timer_idx_t timer_id);

/**
 * @brief Attaches a callback function to a timer interrupt.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer.
 * @param callback The function to call when the timer interrupt occurs.
 * @param arg An optional argument to pass to the callback function.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_AttachInterrupt(timer_group_t group_id, timer_idx_t timer_id, void (*callback)(void*), void* arg);

/**
 * @brief Enables the alarm for a specific timer.
 * The alarm value should have been set during configuration or dynamically before this call.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer.
 * @param enable True to enable the alarm, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_EnableAlarm(timer_group_t group_id, timer_idx_t timer_id, bool enable);

#endif /* HAL_Timer_H */
