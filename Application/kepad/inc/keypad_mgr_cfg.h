/**
 * @file keypad_mgr_cfg.h
 * @brief Configuration definitions for the Keypad Manager (KeypadMgr) module.
 *
 * This file defines physical keypad layout, debounce times, and mapping
 * for buttons to System Manager events.
 */

#ifndef KEYPAD_MGR_CFG_H
#define KEYPAD_MGR_CFG_H

#include "keypad_mgr.h" // For Keypad_Button_ID_t and other types

// --- Physical Keypad Layout ---
#define KEYPAD_NUM_ROWS    (4) /**< Number of rows in the keypad matrix */
#define KEYPAD_NUM_COLUMNS (4) /**< Number of columns in the keypad matrix */

// --- GPIO Pin Assignments (Specific to your hardware) ---
// Keypad Rows
#define KEYPAD_ROW1_GPIO (4)
#define KEYPAD_ROW2_GPIO (12)
#define KEYPAD_ROW3_GPIO (13)
#define KEYPAD_ROW4_GPIO (0)
// Array for easy iteration in HAL mocks/drivers
#define KEYPAD_ROW_GPIOS {KEYPAD_ROW1_GPIO, KEYPAD_ROW2_GPIO, KEYPAD_ROW3_GPIO, KEYPAD_ROW4_GPIO}

// Keypad Columns
#define KEYPAD_COL1_GPIO (36)
#define KEYPAD_COL2_GPIO (37)
#define KEYPAD_COL3_GPIO (38)
#define KEYPAD_COL4_GPIO (39)
// Array for easy iteration in HAL mocks/drivers
#define KEYPAD_COL_GPIOS {KEYPAD_COL1_GPIO, KEYPAD_COL2_GPIO, KEYPAD_COL3_GPIO, KEYPAD_COL4_GPIO}


typedef enum
{
    Keypad_Event_ID_0 = 0, ///< 
    Keypad_Event_ID_1,     ///< 
    Keypad_Event_ID_2,     ///< 
    Keypad_Event_ID_3,     ///< 
    Keypad_Event_ID_4,     ///< 
    Keypad_Event_ID_5,     ///< 
    Keypad_Event_ID_6,     ///< 
    Keypad_Event_ID_7,     ///< 
    Keypad_Event_ID_8,     ///< 
    Keypad_Event_ID_9,     ///< 
    Keypad_Event_ID_10,     ///< 
    Keypad_Event_ID_11,     ///< 
    Keypad_Event_ID_12,     ///< 
    Keypad_Event_ID_13,     ///< 
    Keypad_Event_ID_14,     ///< 
    Keypad_Event_ID_15,     ///< 
    Keypad_Event_MAX,      ///< Total number of configured Events sensors
    Keypad_Event_NONE
} keypad_Event_ID_t;



// --- Debounce and Hold Times ---
#define KEYPAD_SCAN_PERIOD_MS (50) /**< Period at which KeypadMgr_MainFunction is called */
#define KEYPAD_DEBOUNCE_TICKS (2)  /**< Number of scan periods for debounce (e.g., 2 * 50ms = 100ms) */
#define KEYPAD_HOLD_TICKS     (20) /**< Number of scan periods for hold event (e.g., 20 * 50ms = 1000ms = 1s) */

// --- Button to System Manager Event Mapping ---
/**
 * @brief Structure to define how a physical button maps to a System Manager event.
 */
typedef struct
{
    Keypad_Button_ID_t button_id;   /**< The physical button ID */
    keypad_Event_ID_t event_id;   /**< The corresponding System Manager event */
    bool is_mode_select_button;     /**< True if this button directly changes SysMgr mode */
} Keypad_SysMgr_Event_Mapping_t;

/**
 * @brief External declaration for the array mapping physical buttons to System Manager events.
 * This array is defined in keypad_mgr_cfg.c.
 */
extern const Keypad_SysMgr_Event_Mapping_t g_keypad_sys_mgr_event_map[];

// Define the size of the mapping array
extern const uint8_t g_keypad_sys_mgr_event_map_size;



#endif // KEYPAD_MGR_CFG_H