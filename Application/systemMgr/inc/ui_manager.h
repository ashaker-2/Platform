/**
 * @file ui_manager.h
 * @brief User Interface Manager Public API
 * @version 2.2
 * @date 2025
 *
 * This header defines the public interface for the User Interface Manager
 * component. This includes the initialization function, the main periodic
 * function, and any shared constants or type definitions.
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <stdint.h>
#include "common.h"
#include "sys_mgr.h"
#include "ui_manager_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * TYPE DEFINITIONS
 * ============================================================================= */

/**
 * @brief UI State Machine States
 */
typedef enum {
    UI_STATE_MAIN_SCREEN = 0,      /**< Rotating through main monitoring screens */
    UI_STATE_MENU_ROOT,            /**< Main configuration menu */
    UI_STATE_EDIT_GLOBAL_TEMP,     /**< Editing global temperature thresholds */
    UI_STATE_EDIT_GLOBAL_HUM,      /**< Editing global humidity thresholds */
    UI_STATE_CONFIG_SETS,          /**< Configuring per-sensor sets */
    UI_STATE_SELECT_MODE,          /**< Selecting system operating mode */
    UI_STATE_ACTUATOR_TIMERS,      /**< Configuring time-based actuator cycles */
    UI_STATE_LIGHT_SCHEDULE,       /**< Configuring light schedule */
    UI_STATE_SAVE_AND_EXIT,        /**< Saving configuration and exiting menu */
    UI_STATE_COUNT
} UI_State_t;


/* =============================================================================
 * PUBLIC API
 * ============================================================================= */

/**
 * @brief Initializes the User Interface Manager.
 *
 * This function sets up the display and the initial state. It is idempotent.
 */
void UI_MGR_Init(void);

/**
 * @brief The main periodic function for the UI Manager.
 *
 * This function handles screen rotation and polls the keypad event queue.
 * It is a non-blocking function designed to be called repeatedly in the
 * main loop or from a dedicated RTOS task.
 */
void UI_MGR_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H */