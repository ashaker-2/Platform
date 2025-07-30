// ecual/inc/ecual_timer.h

#ifndef ECUAL_TIMER_H
#define ECUAL_TIMER_H

#include <stdint.h> // For uint8_t, uint64_t
#include <stdbool.h> // For bool

// IMPORTANT: Directly include the MCAL driver header to align enum values.
#include "driver/timer.h" // Provides TIMER_GROUP_0, TIMER_0, TIMER_COUNT_UP, etc.

// Forward declaration of ECUAL_TIMER_ID_t from config file
typedef enum ECUAL_TIMER_ID_t ECUAL_TIMER_ID_t;

/**
 * @brief Defines the Timer Group.
 * Values are directly aligned with ESP-IDF's timer_group_t.
 */
typedef enum {
    ECUAL_TIMER_GROUP_0 = TIMER_GROUP_0, ///< Timer Group 0
    ECUAL_TIMER_GROUP_1 = TIMER_GROUP_1  ///< Timer Group 1
} ECUAL_TIMER_Group_t;

/**
 * @brief Defines the Timer Index within a group.
 * Values are directly aligned with ESP-IDF's timer_idx_t.
 */
typedef enum {
    ECUAL_TIMER_INDEX_0 = TIMER_0, ///< Timer 0 within the group
    ECUAL_TIMER_INDEX_1 = TIMER_1  ///< Timer 1 within the group
} ECUAL_TIMER_Index_t;

/**
 * @brief Defines the Timer counting direction.
 * Values are directly aligned with ESP-IDF's timer_count_dir_t.
 */
typedef enum {
    ECUAL_TIMER_COUNT_UP   = TIMER_COUNT_UP,   ///< Timer counts up
    ECUAL_TIMER_COUNT_DOWN = TIMER_COUNT_DOWN  ///< Timer counts down
} ECUAL_TIMER_CountDirection_t;

/**
 * @brief Defines the Timer auto-reload mode.
 * Values are directly aligned with ESP-IDF's timer_autoreload_t.
 */
typedef enum {
    ECUAL_TIMER_AUTORELOAD_DISABLE = TIMER_AUTORELOAD_DIS, ///< Auto-reload disabled
    ECUAL_TIMER_AUTORELOAD_ENABLE  = TIMER_AUTORELOAD_EN   ///< Auto-reload enabled on alarm
} ECUAL_TIMER_AutoReload_t;

/**
 * @brief Function pointer type for timer callbacks.
 * This function will be called when the timer alarm triggers.
 * @param arg A user-defined argument passed to the callback.
 */
typedef void (*ECUAL_TIMER_Callback_t)(void *arg);

/**
 * @brief Structure to hold the configuration for a single Timer.
 */
typedef struct {
    ECUAL_TIMER_Group_t        group;           ///< Timer Group (0 or 1)
    ECUAL_TIMER_Index_t        index;           ///< Timer Index within the group (0 or 1)
    uint32_t                   divider;         ///< Clock divider (e.g., 80 for 1MHz, 80000 for 1kHz)
    ECUAL_TIMER_CountDirection_t count_direction; ///< Counting direction (up or down)
    uint64_t                   initial_count;   ///< Initial counter value
    bool                       enable_alarm;    ///< Enable alarm feature
    uint64_t                   alarm_value;     ///< Value at which alarm triggers
    ECUAL_TIMER_AutoReload_t   auto_reload;     ///< Auto-reload on alarm
    bool                       enable_interrupt;///< Enable interrupt for this timer
    // For interrupt, a default callback (or placeholder) can be defined
    ECUAL_TIMER_Callback_t     callback;        ///< Callback function for timer interrupt
    void                       *callback_arg;   ///< Argument for the callback function
} ECUAL_TIMER_Config_t;

/**
 * @brief Internal structure to map ECUAL_TIMER_ID_t to Timer Group/Index.
 * Used internally by ecual_timer.c.
 */
typedef struct {
    ECUAL_TIMER_Group_t group;
    ECUAL_TIMER_Index_t index;
} ECUAL_TIMER_Internal_Map_t;


/**
 * @brief Initializes all Timers based on the configurations defined in ecual_timer_config.h.
 * @return ECUAL_OK if all timers are initialized successfully, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_TIMER_Init(void);

/**
 * @brief Starts a specific timer.
 * @param timer_id The ECUAL_TIMER_ID_t of the timer to start.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_TIMER_Start(ECUAL_TIMER_ID_t timer_id);

/**
 * @brief Stops a specific timer.
 * @param timer_id The ECUAL_TIMER_ID_t of the timer to stop.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_TIMER_Stop(ECUAL_TIMER_ID_t timer_id);

/**
 * @brief Gets the current counter value of a specific timer.
 * @param timer_id The ECUAL_TIMER_ID_t of the timer.
 * @param count Pointer to a uint64_t variable to store the count.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_TIMER_GetCount(ECUAL_TIMER_ID_t timer_id, uint64_t *count);

/**
 * @brief Sets the counter value of a specific timer.
 * @param timer_id The ECUAL_TIMER_ID_t of the timer.
 * @param count The new counter value.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_TIMER_SetCount(ECUAL_TIMER_ID_t timer_id, uint64_t count);

/**
 * @brief Registers a callback function for a specific timer's alarm interrupt.
 * This overwrites any previously registered callback for that timer.
 * @param timer_id The ECUAL_TIMER_ID_t of the timer.
 * @param callback The function to be called on alarm. Pass NULL to unregister.
 * @param arg An argument to be passed to the callback function.
 * @return ECUAL_OK if successful, ECUAL_ERROR if timer_id is invalid.
 */
uint8_t ECUAL_TIMER_RegisterCallback(ECUAL_TIMER_ID_t timer_id, ECUAL_TIMER_Callback_t callback, void *arg);

#endif /* ECUAL_TIMER_H */
