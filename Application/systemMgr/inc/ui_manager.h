/**
 * @file ui_manager.h
 * @brief User Interface Manager Public API
 * @version 2.3
 * @date 2025
 *
 * The UI Manager implements a state-machine-driven user interface
 * for the System Manager. It handles keypad input, manages menus,
 * and displays sensor/system information on a 2×16 LCD.
 *
 * Responsibilities:
 *  - Display rotating main screens (dashboard)
 *  - Provide configuration menus for thresholds and actuators
 *  - Handle keypad events (press, hold, release)
 *  - Timeout management for auto-return to main screen
 *
 * The UI Manager is **non-blocking** and must be called periodically
 * from the system main loop or a dedicated RTOS task.
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
 * UI STATE DEFINITIONS
 * ============================================================================= */

/**
 * @brief UI state machine states.
 *
 * These represent the "screens" or "contexts" the user can navigate through.
 * Transitions are handled by keypad input and timeout logic.
 */
typedef enum {
    UI_STATE_MAIN_SCREEN = 0,   /**< Rotating main dashboard screens */
    UI_STATE_MENU_ROOT,         /**< Root configuration menu */
    UI_STATE_EDIT_GLOBAL_TEMP,  /**< Editing global temperature thresholds */
    UI_STATE_EDIT_GLOBAL_HUM,   /**< Editing global humidity thresholds */
    UI_STATE_CONFIG_FAN,        /**< Configure fan cycle/manual/auto */
    UI_STATE_CONFIG_VENTS,       /**< Configure ventilator cycle/manual/auto */
    UI_STATE_CONFIG_PUMP,       /**< Configure pump cycle/manual/auto */
    UI_STATE_CONFIG_HEATER,     /**< Configure heater cycle/manual/auto */
    UI_STATE_CONFIG_LIGHT,      /**< Configure light on/off schedule */
    UI_STATE_CONFIG_SENSOR,     /**< Placeholder: per-sensor config (future use) */
    UI_STATE_SAVE_AND_EXIT,     /**< Saving configuration and exiting menu */
    UI_STATE_COUNT
} UI_State_t;


/* Dashboard pages */
typedef enum {
    DASH_PAGE_AVG = 0,
    DASH_PAGE_SENSOR,
    DASH_PAGE_ACTUATORS,
    DASH_PAGE_MODETIME,
    DASH_PAGE_COUNT
} DashPage_t;

/* Units when editing actuator times */
typedef enum {
    UNIT_SEC = 0,
    UNIT_MIN,
    UNIT_HOUR,
    UNIT_COUNT
} TimeUnit_t;


/* Actuator edit context */
typedef struct {
    SYS_MGR_Actuator_t type;   /* actuator type being edited */
    uint16_t id;               /* actuator id (0-based) */
    bool manual;               /* manual vs auto selected */
    uint32_t on_seconds;       /* canonical seconds */
    uint32_t off_seconds;
    TimeUnit_t unit_on;        /* selected unit display for on */
    TimeUnit_t unit_off;
} ActuatorEditCtx_t;


/* Light edit context */
typedef struct {
    uint8_t on_h, on_m;
    uint8_t off_h, off_m;
} LightEditCtx_t;

/* =============================================================================
 * PUBLIC API
 * ============================================================================= */

/**
 * @brief Initialize the User Interface Manager.
 *
 * - Clears the LCD
 * - Initializes state machine to main screen
 * - Loads configuration snapshot from SysMgr
 * - Displays the initial dashboard screen
 */
void UI_MGR_Init(void);

/**
 * @brief Periodic main function for UI Manager.
 *
 * This must be called at a fixed interval (`UI_MGR_MAIN_PERIOD_MS`).
 *
 * Responsibilities:
 *  - Poll keypad events and dispatch them to handlers
 *  - Update display content (main screen rotation, menu screens)
 *  - Handle inactivity timeout (return to dashboard if idle)
 */
void UI_MGR_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H */
