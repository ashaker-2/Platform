/**
 * @file hal_timer.h
 * @brief Hardware Abstraction Layer for Timer - Interface Header.
 *
 * This header defines the public API for the Timer HAL, providing an
 * abstract interface for timer functionalities such as initialization,
 * starting/stopping timers, setting periods, and registering callbacks.
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for Timer HAL status codes.
 */
typedef enum
{
    HAL_TIMER_STATUS_OK = 0,             /**< Operation successful. */
    HAL_TIMER_STATUS_ERROR,              /**< Generic error. */
    HAL_TIMER_STATUS_INVALID_PARAM,      /**< Invalid parameter provided. */
    HAL_TIMER_STATUS_NOT_INITIALIZED,    /**< Timer HAL not initialized. */
    HAL_TIMER_STATUS_ALREADY_INITIALIZED,/**< Timer HAL already initialized. */
    HAL_TIMER_STATUS_BUSY,               /**< Timer is busy with an operation. */
    HAL_TIMER_STATUS_MAX                 /**< Maximum number of status codes. */
} HAL_TIMER_Status_t;

/**
 * @brief Enumeration for available Timer channels.
 *
 * These map to underlying hardware timers (e.g., ESP32's Timer Group 0, Timer 0/1).
 */
typedef enum
{
    HAL_TIMER_CHANNEL_0 = 0,    /**< Timer Channel 0 (e.g., TG0_T0 on ESP32). */
    HAL_TIMER_CHANNEL_1,        /**< Timer Channel 1 (e.g., TG0_T1 on ESP32). */
    // Add more channels if needed, mapping to other timer groups/indices
    HAL_TIMER_CHANNEL_MAX       /**< Maximum number of timer channels. */
} HAL_TIMER_Channel_t;

/**
 * @brief Enumeration for Timer operation modes.
 */
typedef enum
{
    HAL_TIMER_MODE_ONE_SHOT = 0,/**< Timer runs once and stops after reaching its period. */
    HAL_TIMER_MODE_AUTO_RELOAD, /**< Timer reloads its period and continues counting after expiry. */
    HAL_TIMER_MODE_MAX          /**< Maximum number of timer modes. */
} HAL_TIMER_Mode_t;

/**
 * @brief Enumeration for Timer counting direction.
 */
typedef enum
{
    HAL_TIMER_DIR_UP = 0,       /**< Timer counts up from 0 to period. */
    HAL_TIMER_DIR_DOWN,         /**< Timer counts down from period to 0. */
    HAL_TIMER_DIR_MAX           /**< Maximum number of timer directions. */
} HAL_TIMER_Direction_t;

/**
 * @brief Callback function type for timer expiry events.
 *
 * This function will be called when a configured timer reaches its period.
 *
 * @param channel The timer channel that triggered the callback.
 * @param user_data Optional user-defined data passed during callback registration.
 */
typedef void (*HAL_TIMER_Callback_t)(HAL_TIMER_Channel_t channel, void *user_data);

/**
 * @brief Initializes a specific timer channel.
 *
 * This function configures the base parameters of a timer channel,
 * including its mode, direction, and prescaler. It should be called
 * before starting the timer.
 *
 * @param channel The timer channel to initialize.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_Init(HAL_TIMER_Channel_t channel);

/**
 * @brief De-initializes a specific timer channel.
 *
 * This function stops the timer, disables its interrupt, and frees
 * associated resources.
 *
 * @param channel The timer channel to de-initialize.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_DeInit(HAL_TIMER_Channel_t channel);

/**
 * @brief Starts a specific timer channel.
 *
 * The timer will begin counting based on its configured mode and period.
 *
 * @param channel The timer channel to start.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_Start(HAL_TIMER_Channel_t channel);

/**
 * @brief Stops a specific timer channel.
 *
 * The timer will cease counting, and its counter value will be retained.
 *
 * @param channel The timer channel to stop.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_Stop(HAL_TIMER_Channel_t channel);

/**
 * @brief Sets the period (alarm value) for a specific timer channel.
 *
 * This function sets the target count value at which the timer will trigger
 * an event (and potentially reload).
 *
 * @param channel The timer channel to configure.
 * @param period_us The period in microseconds. The actual period might be
 * slightly different due to prescaler and clock limitations.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_SetPeriod(HAL_TIMER_Channel_t channel, uint64_t period_us);

/**
 * @brief Gets the current counter value of a specific timer channel.
 *
 * @param channel The timer channel to query.
 * @param value Pointer to a uint64_t where the current counter value will be stored.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_GetCounterValue(HAL_TIMER_Channel_t channel, uint64_t *value);

/**
 * @brief Registers a callback function for a specific timer channel's expiry event.
 *
 * This callback will be invoked when the timer reaches its configured period.
 *
 * @param channel The timer channel for which to register the callback.
 * @param callback The function to be called on timer expiry. Can be NULL to unregister.
 * @param user_data Optional user-defined data to be passed to the callback function.
 * @return HAL_TIMER_STATUS_OK if successful, an error code otherwise.
 */
HAL_TIMER_Status_t HAL_TIMER_RegisterCallback(HAL_TIMER_Channel_t channel, HAL_TIMER_Callback_t callback, void *user_data);

#endif /* HAL_TIMER_H */
