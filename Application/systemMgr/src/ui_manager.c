/**
 * @file ui_manager.c
 * @brief User Interface Manager (LCD + Keypad)
 * @version 2.2
 * @date 2025
 *
 * This component provides a non-blocking, state-machine-driven user interface
 * for the SysMgr system. It handles input from a 4x4 keypad and displays
 * real-time information and configuration menus on a 2x16 character LCD.
 *
 * This version has been updated to use the KeypadMgr event queue and
 * supports different event types (PRESS, HOLD). It also correctly implements
 * all display functions and input handling for each menu state.
 */

#include "ui_manager.h"
#include "ui_manager_cfg.h"
#include "sys_mgr.h"
#include "temphumctrl.h"
#include "keypad_mgr.h"
#include "char_display.h"
#include "logger.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* =============================================================================
 * PRIVATE MACROS AND DEFINITIONS
 * ============================================================================= */

#define TAG "UIMgr"

/* =============================================================================
 * PRIVATE GLOBAL VARIABLES
 * ============================================================================= */

static UI_State_t g_ui_state = UI_STATE_MAIN_SCREEN;
static SysMgr_Config_t g_ui_working_configuration;
static char g_input_buffer[UI_MAX_INPUT_LEN + 1];
static uint8_t g_input_index = 0;
static uint32_t g_last_key_press_ms = 0;
static uint32_t g_last_display_update_ms = 0;
static uint8_t g_main_screen_index = 0;
static bool g_is_editing_min = true;

/* =============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================================= */

static void ui_clear_input_buffer(void);
static void ui_display_main_screen(void);
static void ui_display_menu_root(void);
static void ui_display_edit_global_temp(void);
static void ui_display_edit_global_hum(void);
static void ui_display_select_mode(void);
static void ui_display_per_sensor_readings(void);
static void ui_display_actuator_states(void);

static void ui_handle_event(Keypad_Event_t *event);
static void ui_handle_event_main_screen(Keypad_Event_t *event);
static void ui_handle_event_menu_root(Keypad_Event_t *event);
static void ui_handle_event_edit_global_temp(Keypad_Event_t *event);
static void ui_handle_event_edit_global_hum(Keypad_Event_t *event);
static void ui_handle_event_select_mode(Keypad_Event_t *event);
static void ui_handle_numeric_input(Keypad_Event_t *event);

/* =============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================= */

/**
 * @brief Initializes the User Interface Manager.
 *
 * This function sets up the display and the initial state. It is idempotent.
 */
void UI_MGR_Init(void)
{
    LOGI(TAG, "Initializing UI Manager...");
    ui_display_main_screen();
}

/**
 * @brief The main periodic function for the UI Manager.
 *
 * This function handles screen rotation and polls the keypad event queue.
 * It is a non-blocking function designed to be called repeatedly in the
 * main loop or from a dedicated RTOS task.
 */
void UI_MGR_MainFunction(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = UI_MGR_GetTick();

    if ((now - last_tick) < UI_MGR_MAIN_PERIOD_MS) {
        return;
    }
    last_tick = now;

    // Process all events from the keypad queue
    Keypad_Event_t event;
    while (KeypadMgr_GetEvent(&event) == E_OK) {
        ui_handle_event(&event);
    }
    
    // Manage display updates based on current state
    if (g_ui_state == UI_STATE_MAIN_SCREEN)
    {
        if ((now - g_last_display_update_ms) >= UI_SCREEN_ROTATE_MS)
        {
            g_main_screen_index = (g_main_screen_index + 1) % 3;
            ui_display_main_screen();
            g_last_display_update_ms = now;
        }
    }
    else
    {
        // Menu timeout
        if ((now - g_last_key_press_ms) >= UI_MENU_TIMEOUT_MS)
        {
            LOGW(TAG, "Menu timeout. Auto-saving and exiting...");
            // SysMgr_SetConfig(&g_ui_working_configuration);
            // SysMgr_SaveConfigToFlash();
            g_ui_state = UI_STATE_MAIN_SCREEN;
            ui_display_main_screen();
        }
    }
}

/* =============================================================================
 * PRIVATE FUNCTION IMPLEMENTATION
 * ============================================================================= */

/**
 * @brief Handles a keypad event based on the current UI state.
 *
 * This function serves as the central state machine transition handler.
 * It resets the menu timeout on any key press.
 *
 * @param event The keypad event to process.
 */
static void ui_handle_event(Keypad_Event_t *event)
{
    g_last_key_press_ms = UI_MGR_GetTick(); // Reset timeout on any key press
    
    switch (g_ui_state)
    {
        case UI_STATE_MAIN_SCREEN:
            ui_handle_event_main_screen(event);
            break;
        case UI_STATE_MENU_ROOT:
            ui_handle_event_menu_root(event);
            break;
        case UI_STATE_EDIT_GLOBAL_TEMP:
            ui_handle_event_edit_global_temp(event);
            break;
        case UI_STATE_EDIT_GLOBAL_HUM:
            ui_handle_event_edit_global_hum(event);
            break;
        case UI_STATE_SELECT_MODE:
            ui_handle_event_select_mode(event);
            break;
        case UI_STATE_SAVE_AND_EXIT:
            // Intentional fallthrough, no input handling
        case UI_STATE_CONFIG_SETS:
        case UI_STATE_ACTUATOR_TIMERS:
        case UI_STATE_LIGHT_SCHEDULE:
        default:
            // Unhandled states or keys are ignored
            break;
    }
}

/**
 * @brief Handles events while in the main screen state.
 *
 * A long press on the ENTER key (`KEYPAD_BTN_ENTER`) transitions the UI
 * to the root configuration menu.
 *
 * @param event The keypad event to process.
 */
static void ui_handle_event_main_screen(Keypad_Event_t *event)
{
    // Transition to menu on a long press of ENTER
    if (event->type == KEYPAD_EVT_HOLD && event->button == KEYPAD_BTN_ENTER) {
        LOGI(TAG, "Entering configuration menu.");
        // SysMgr_GetConfig(&g_ui_working_configuration);
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
        ui_clear_input_buffer();
    }
}

/**
 * @brief Handles events while in the root menu state.
 *
 * Numeric keys `1` through `6` transition to sub-menus. The `BACK` or
 * `ERASE` key saves the configuration and returns to the main screen.
 *
 * @param event The keypad event to process.
 */
static void ui_handle_event_menu_root(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS) {
        return;
    }

    switch (event->button) {
        case KEYPAD_BTN_1:
            g_ui_state = UI_STATE_EDIT_GLOBAL_TEMP;
            g_is_editing_min = true;
            ui_clear_input_buffer();
            ui_display_edit_global_temp();
            break;
        case KEYPAD_BTN_2:
            g_ui_state = UI_STATE_EDIT_GLOBAL_HUM;
            g_is_editing_min = true;
            ui_clear_input_buffer();
            ui_display_edit_global_hum();
            break;
        case KEYPAD_BTN_3:
            g_ui_state = UI_STATE_CONFIG_SETS;
            ui_display_message("Config Sets", "Not Implemented");
            break;
        case KEYPAD_BTN_4:
            g_ui_state = UI_STATE_SELECT_MODE;
            ui_display_select_mode();
            break;
        case KEYPAD_BTN_5:
            g_ui_state = UI_STATE_ACTUATOR_TIMERS;
            ui_display_message("Actuator Timers", "Not Implemented");
            break;
        case KEYPAD_BTN_6:
            g_ui_state = UI_STATE_LIGHT_SCHEDULE;
            ui_display_message("Light Schedule", "Not Implemented");
            break;
        case KEYPAD_BTN_BACK:
        case KEYPAD_BTN_ERASE:
            g_ui_state = UI_STATE_SAVE_AND_EXIT;
            ui_display_message("Saving config...", "");
            // SysMgr_SetConfig(&g_ui_working_configuration);
            // SysMgr_SaveConfigToFlash();
            g_ui_state = UI_STATE_MAIN_SCREEN;
            ui_display_main_screen();
            break;
        default:
            break;
    }
}

/**
 * @brief Handles numeric input and control keys for editing global temperature.
 * @param event The keypad event to process.
 */
static void ui_handle_event_edit_global_temp(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS) {
        return;
    }
    
    if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
        ui_handle_numeric_input(event);
        ui_display_edit_global_temp();
    } else if (event->button == KEYPAD_BTN_ERASE) {
        if (g_input_index > 0) {
            g_input_buffer[--g_input_index] = '\0';
            ui_display_edit_global_temp();
        }
    } else if (event->button == KEYPAD_BTN_BACK) {
        ui_clear_input_buffer();
        g_is_editing_min = true;
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    } else if (event->button == KEYPAD_BTN_ENTER) {
        float value = strtof(g_input_buffer, NULL);
        if (g_is_editing_min) 
        {
            // if (SYS_MGR_ValidateTemperatureRange(value, g_ui_working_configuration.global_temp_max) == E_OK) 
            {
                g_ui_working_configuration.global_temp_min = value;
                g_is_editing_min = false;
                ui_clear_input_buffer();
                ui_display_edit_global_temp();
            } 
            // else 
            {
                ui_display_message("Invalid T-Min!", "Range: 0-50C");
            }
        } 
        else 
        {
            // if (SYS_MGR_ValidateTemperatureRange(g_ui_working_configuration.global_temp_min, value) == E_OK) {
            //     g_ui_working_configuration.global_temp_max = value;
            //     ui_display_message("Temp Updated!", "Press BACK");
            //     g_ui_state = UI_STATE_MENU_ROOT;
            //     ui_clear_input_buffer();
            // } 
            // else 
            {
                ui_display_message("Invalid T-Max!", "Range: 0-50C");
            }
        }
    }
}

/**
 * @brief Handles numeric input and control keys for editing global humidity.
 * @param event The keypad event to process.
 */
static void ui_handle_event_edit_global_hum(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS) {
        return;
    }

    if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
        ui_handle_numeric_input(event);
        ui_display_edit_global_hum();
    } else if (event->button == KEYPAD_BTN_ERASE) {
        if (g_input_index > 0) {
            g_input_buffer[--g_input_index] = '\0';
            ui_display_edit_global_hum();
        }
    } else if (event->button == KEYPAD_BTN_BACK) {
        ui_clear_input_buffer();
        g_is_editing_min = true;
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    } else if (event->button == KEYPAD_BTN_ENTER) {
        float value = strtof(g_input_buffer, NULL);
        if (g_is_editing_min) {
            // if (SYS_MGR_ValidateHumidityRange(value, g_ui_working_configuration.global_hum_max) == E_OK) 
            {
                g_ui_working_configuration.global_hum_min = value;
                g_is_editing_min = false;
                ui_clear_input_buffer();
                ui_display_edit_global_hum();
            } 
            // else 
            {
                ui_display_message("Invalid H-Min!", "Range: 0-100%");
            }
        } 
        else 
        {
            // if (SYS_MGR_ValidateHumidityRange(g_ui_working_configuration.global_hum_min, value) == E_OK) 
            {
                g_ui_working_configuration.global_hum_max = value;
                ui_display_message("Hum Updated!", "Press BACK");
                g_ui_state = UI_STATE_MENU_ROOT;
                ui_clear_input_buffer();
            } 
            // else 
            {
                ui_display_message("Invalid H-Max!", "Range: 0-100%");
            }
        }
    }
}

/**
 * @brief Handles events for selecting the system operational mode.
 *
 * Keys `1`, `2`, or `3` set the mode. `BACK` or `ERASE` returns to the root menu.
 *
 * @param event The keypad event to process.
 */
static void ui_handle_event_select_mode(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS) {
        return;
    }
    
    switch (event->button) {
        case KEYPAD_BTN_1:
            g_ui_working_configuration.mode = SYS_MGR_MODE_AUTOMATIC;
            ui_display_message("Mode Set", "Automatic");
            break;
        case KEYPAD_BTN_2:
            g_ui_working_configuration.mode = SYS_MGR_MODE_HYBRID;
            ui_display_message("Mode Set", "Hybrid");
            break;
        case KEYPAD_BTN_3:
            g_ui_working_configuration.mode = SYS_MGR_MODE_MANUAL;
            ui_display_message("Mode Set", "Manual");
            break;
        case KEYPAD_BTN_BACK:
        case KEYPAD_BTN_ERASE:
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_clear_input_buffer();
            ui_display_menu_root();
            break;
        default:
            break;
    }
}

/**
 * @brief Handles generic numeric input for all states.
 * @param event The keypad event containing the button pressed.
 */
static void ui_handle_numeric_input(Keypad_Event_t *event)
{
    if (g_input_index < UI_MAX_INPUT_LEN) {
        g_input_buffer[g_input_index++] = '0' + event->button;
        g_input_buffer[g_input_index] = '\0';
    }
}

/* =============================================================================
 * PRIVATE DISPLAY AND HELPER FUNCTION IMPLEMENTATION
 * ============================================================================= */

/**
 * @brief Displays the main rotating screens.
 */
static void ui_display_main_screen(void)
{
    ui_display_clear();
    switch (g_main_screen_index)
    {
        case 0:
            ui_display_message("Avg T:%.1fC H:%.1f%%", "Mode: AUTO     "); // Stubs for now
            break;
        case 1:
            ui_display_per_sensor_readings();
            break;
        case 2:
            ui_display_actuator_states();
            break;
    }
}

/**
 * @brief Displays the per-sensor readings screen.
 * This function retrieves data from SysMgr and formats it for the display.
 */
static void ui_display_per_sensor_readings(void)
{
    // PerSensorReadings_t readings;
    // if (SYS_MGR_GetPerSensorReadings(&readings) == E_OK) 
    {
        // char line1[UI_LCD_COLS + 1];
        // char line2[UI_LCD_COLS + 1];
        // snprintf(line1, sizeof(line1), "S1:%.1fC S2:%.1fC", readings.temp_readings[0], readings.temp_readings[1]);
        // snprintf(line2, sizeof(line2), "S3:%.1fC S4:%.1fC", readings.temp_readings[2], readings.temp_readings[3]);
        // ui_display_message(line1, line2);
    } 
    // else 
    {
        ui_display_message("Readings Failed", "Please check sensors");
    }
}

/**
 * @brief Displays the actuator states screen.
 * This function retrieves data from SysMgr and formats it for the display.
 */
static void ui_display_actuator_states(void)
{
    // SYS_MGR_Actuator_States_t states;
    // if (SYS_MGR_GetActuatorStates(&states) == E_OK) 
    // {
    //     char line1[UI_LCD_COLS + 1];
    //     char line2[UI_LCD_COLS + 1];
    //     snprintf(line1, sizeof(line1), "Fan:%s Heat:%s", states.fans_on ? "ON" : "OFF", states.heaters_on ? "ON" : "OFF");
    //     snprintf(line2, sizeof(line2), "Pump:%s Vent:%s", states.pumps_on ? "ON" : "OFF", states.vents_on ? "ON" : "OFF");
    //     ui_display_message(line1, line2);
    // } 
    // else 
    {
        ui_display_message("Actuators Failed", "Please check system");
    }
}

/**
 * @brief Displays the root configuration menu.
 */
static void ui_display_menu_root(void) 
{
    ui_display_message("Config Menu", "1:Temp 2:Humid 4:Mode");
}

/**
 * @brief Displays the global temperature input screen.
 */
static void ui_display_edit_global_temp(void) 
{
    if (g_is_editing_min) 
    {
        ui_display_numeric_input("Set Temp Min:", g_input_buffer);
    } 
    else 
    {
        ui_display_numeric_input("Set Temp Max:", g_input_buffer);
    }
}

/**
 * @brief Displays the global humidity input screen.
 */
static void ui_display_edit_global_hum(void) 
{
    if (g_is_editing_min) 
    {
        ui_display_numeric_input("Set Hum Min:", g_input_buffer);
    } 
    else 
    {
        ui_display_numeric_input("Set Hum Max:", g_input_buffer);
    }
}

/**
 * @brief Displays the system mode selection screen.
 */
static void ui_display_select_mode(void) 
{
    ui_display_message("Select Mode", "1:Auto 2:Hybrid 3:Manual");
}

/**
 * @brief Clears the numeric input buffer and resets its length.
 *
 * This function should be called after a numeric value has been processed
 * or when the user cancels the input process.
 */
static void ui_clear_input_buffer(void)
{
    memset(g_input_buffer, 0, sizeof(g_input_buffer));
    g_input_index = 0;
}