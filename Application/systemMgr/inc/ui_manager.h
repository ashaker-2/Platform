/**
 * @file ui_manager.h
 * @brief User Interface Manager Public API
 * @version 2.1
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

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * CONSTANTS
 * ============================================================================= */

/** @defgroup UIMgr_Constants UI Manager Constants
 * @{
 */

/** Screen rotation period in milliseconds */
#define UI_SCREEN_ROTATION_PERIOD_MS    3000

/** Menu timeout in milliseconds (auto-exit) */
#define UI_MENU_TIMEOUT_MS              60000

/** Maximum input digits for numeric entry */
#define UI_MAX_INPUT_DIGITS             3

/** Main function execution period in milliseconds */
#define UI_MGR_MAIN_PERIOD_MS          100

/** LCD display dimensions */
#define UI_LCD_ROWS                     2
#define UI_LCD_COLS                     16

/** @} */

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

/**
 * @brief Main screen types (rotating display)
 */
typedef enum {
    UI_MAIN_SCREEN_AVG_READINGS = 0,  /**< Average temperature and humidity */
    UI_MAIN_SCREEN_PER_SENSOR,        /**< Per-sensor readings */
    UI_MAIN_SCREEN_ACTUATORS,         /**< Actuator states */
    UI_MAIN_SCREEN_COUNT
} UI_MainScreen_t;

/**
 * @brief Configuration menu options
 */
typedef enum {
    UI_MENU_GLOBAL_TEMP = 1,      /**< Set global temperature range */
    UI_MENU_GLOBAL_HUM = 2,       /**< Set global humidity range */
    UI_MENU_SETS_CONFIG = 3,      /**< Enable/disable sensor sets */
    UI_MENU_PER_SENSOR_TEMP = 4,  /**< Per-sensor temperature config */
    UI_MENU_PER_SENSOR_HUM = 5,   /**< Per-sensor humidity config */
    UI_MENU_MODE_SELECT = 6,      /**< System mode selection */
    UI_MENU_ACTUATOR_TIMERS = 7,  /**< Actuator timing configuration */
    UI_MENU_LIGHT_SCHEDULE = 8,   /**< Light schedule configuration */
    UI_MENU_EXIT_SAVE = 9,        /**< Exit and save configuration */
    UI_MENU_COUNT
} UI_MenuOption_t;

/**
 * @brief Numeric input context
 */
typedef struct {
    UI_MenuOption_t menu_option;  /**< Which menu option is being configured */
    uint8_t sensor_id;            /**< Sensor ID for per-sensor config */
    uint8_t actuator_id;          /**< Actuator ID for timing config */
    uint8_t param_index;          /**< Parameter index (min/max, on/off, etc.) */
    char input_buffer[UI_MAX_INPUT_DIGITS + 1]; /**< Input buffer */
    uint8_t input_length;         /**< Current input length */
} UI_NumericInput_t;

/**
 * @brief Error display information
 */
typedef struct {
    char message[UI_LCD_COLS + 1]; /**< Error message to display */
    uint32_t display_time_ms;      /**< How long to display error */
    uint32_t start_time_ms;        /**< When error display started */
} UI_ErrorDisplay_t;

/**
 * @brief UI Manager internal state
 */
typedef struct {
    UI_State_t current_state;              /**< Current UI state */
    UI_MainScreen_t current_main_screen;   /**< Current main screen type */
    UI_MenuOption_t current_menu_option;   /**< Current menu option */
    SysMgr_Config_t working_config;        /**< Working copy of configuration */
    UI_NumericInput_t numeric_input;       /**< Numeric input state */
    UI_ErrorDisplay_t error_display;       /**< Error display state */
    uint32_t screen_timer_ms;              /**< Timer for screen rotation */
    uint32_t menu_timeout_timer_ms;        /**< Timer for menu timeout */
    bool config_modified;                  /**< Flag if configuration was modified */
    bool lcd_needs_update;                 /**< Flag if LCD needs refresh */
} UI_Manager_State_t;

/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

/**
 * @brief Initializes the UI Manager component.
 *
 * This function must be called once at system startup before using
 * any other UI Manager functions. It sets up the initial state and
 * fetches the current system configuration from the SysMgr module.
 *
 * Implementation notes:
 * - Initialize LCD display
 * - Initialize keypad manager
 * - Get current system configuration
 * - Set initial UI state to main screen
 * - Display startup message
 *
 * @return Status_t E_OK on successful initialization.
 */
Status_t UIMgr_Init(void);

/**
 * @brief Main periodic function for the UI Manager.
 *
 * This function handles screen rotation, keypad input, and menu timeouts.
 * It implements a non-blocking state machine to manage user interactions.
 * It should be called at a regular interval, e.g., every 10-50ms.
 *
 * Implementation notes:
 * - Check for keypad events using KeypadMgr_GetLastEvent()
 * - Handle state machine transitions based on current state and input
 * - Update screen rotation timer and switch screens as needed
 * - Handle menu timeout and auto-save configuration
 * - Update LCD display when needed (lcd_needs_update flag)
 * - Process numeric input validation and conversion
 * - Handle sensor humidity support checking for menu options
 */
void UIMgr_MainFunction(void);

/**
 * @brief Forces an immediate LCD update.
 *
 * Can be called to refresh the display immediately, useful after
 * system mode changes or configuration updates.
 *
 * @return Status_t E_OK on success, E_NOK on display error.
 */
Status_t UIMgr_ForceDisplayUpdate(void);

/**
 * @brief Gets the current UI state.
 *
 * Useful for debugging and diagnostics.
 *
 * @param[out] state Pointer to store the current UI state.
 * @return Status_t E_OK on success, E_NOK if pointer is null.
 */
Status_t UIMgr_GetCurrentState(UI_State_t *state);

/**
 * @brief Displays an error message temporarily.
 *
 * Shows an error message on the LCD for a specified duration,
 * then returns to the previous display.
 *
 * @param[in] message Error message to display (max 16 characters).
 * @param[in] duration_ms How long to display the message in milliseconds.
 * @return Status_t E_OK on success, E_NOK if message is null.
 */
Status_t UIMgr_ShowError(const char *message, uint32_t duration_ms);

/**
 * @brief Checks if the UI is currently in configuration menu.
 *
 * Can be used by other modules to determine if configuration
 * changes are in progress.
 *
 * @param[out] in_menu True if currently in configuration menu, false otherwise.
 * @return Status_t E_OK on success, E_NOK if pointer is null.
 */
Status_t UIMgr_IsInConfigMenu(bool *in_menu);

/* =============================================================================
 * DISPLAY HELPER FUNCTIONS (for implementation)
 * ============================================================================= */

/**
 * @brief Display helper functions - these should be static in implementation
 * but are documented here for reference
 */

/**
 * @brief Displays the main screen with average readings.
 * Format: "Avg T:24.5C H:55%"
 *         "Mode: AUTO     "
 */
// static void UIMgr_DisplayAverageReadings(void);

/**
 * @brief Displays per-sensor readings.
 * Format: "S1:23C 45% S2:25C"
 *         "S3:24C --- S4:26C"
 * Note: "---" for sensors that don't support humidity
 */
// static void UIMgr_DisplayPerSensorReadings(void);

/**
 * @brief Displays actuator states.
 * Format: "Fan:ON Heat:OFF "
 *         "Pump:ON Ven:OFF"
 */
// static void UIMgr_DisplayActuatorStates(void);

/**
 * @brief Displays configuration menu.
 * Format: "Config Menu    "
 *         "1:Temp 2:Humid "
 */
// static void UIMgr_DisplayConfigMenu(void);

/**
 * @brief Displays numeric input screen.
 * Format: "Set Temp Min:  "
 *         "Value: 25_     "
 */
// static void UIMgr_DisplayNumericInput(void);

/**
 * @brief Handles keypad input based on current state.
 * @param key The key that was pressed
 */
// static void UIMgr_HandleKeypadInput(uint8_t key);

/**
 * @brief Validates and applies numeric input.
 * @return Status_t E_OK if input valid and applied, E_NOK otherwise
 */
// static Status_t UIMgr_ValidateAndApplyNumericInput(void);

/**
 * @brief Checks if a sensor supports humidity configuration.
 * @param sensor_id Sensor ID to check
 * @return bool True if humidity config should be enabled
 */
// static bool UIMgr_SensorSupportsHumidity(uint8_t sensor_id);

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H */