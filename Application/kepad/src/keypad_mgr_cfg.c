/**
 * @file keypad_mgr_cfg.c
 * @brief Configuration data definitions for the Keypad Manager (KeypadMgr) module.
 *
 * This file defines the array for mapping physical buttons to System Manager events.
 */

#include "keypad_mgr_cfg.h" // Include the configuration header

// --- Button to System Manager Event Mapping Definition ---
// This array maps each physical button (by its Keypad_Button_ID_t)
// to a corresponding System Manager event.
// Buttons marked 'is_mode_select_button = true' will directly trigger mode changes in SysMgr.
const Keypad_SysMgr_Event_Mapping_t g_keypad_sys_mgr_event_map[] = {
    // Example mappings: Adjust these based on your specific keypad layout and desired behavior
    [KEYPAD_BUTTON_0] = {.button_id = KEYPAD_BUTTON_0, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false}, // Example: No direct SysMgr event
    [KEYPAD_BUTTON_1] = {.button_id = KEYPAD_BUTTON_1, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false},
    [KEYPAD_BUTTON_2] = {.button_id = KEYPAD_BUTTON_2, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false},
    [KEYPAD_BUTTON_3] = {.button_id = KEYPAD_BUTTON_3, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false},

    [KEYPAD_BUTTON_MODE_AUTO] = {.button_id = KEYPAD_BUTTON_MODE_AUTO, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = true},
    [KEYPAD_BUTTON_MODE_HYBRID] = {.button_id = KEYPAD_BUTTON_MODE_HYBRID, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = true},
    [KEYPAD_BUTTON_MODE_MANUAL] = {.button_id = KEYPAD_BUTTON_MODE_MANUAL, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = true},

    [KEYPAD_BUTTON_UP] = {.button_id = KEYPAD_BUTTON_UP, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false},      // Might map to generic menu UP
    [KEYPAD_BUTTON_DOWN] = {.button_id = KEYPAD_BUTTON_DOWN, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false},  // Might map to generic menu DOWN
    [KEYPAD_BUTTON_ENTER] = {.button_id = KEYPAD_BUTTON_ENTER, .event_id = KEYPAD_EVENT_NONE, .is_mode_select_button = false} // Might map to generic menu ENTER
};

// Define the size of the mapping array
const uint8_t g_keypad_sys_mgr_event_map_size = sizeof(g_keypad_sys_mgr_event_map) / sizeof(g_keypad_sys_mgr_event_map[0]);
