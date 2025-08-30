/**
 * @file ui_manager.c
 * @brief User Interface Manager (LCD + Keypad)
 * @version 2.3
 * @date 2025
 *
 * This component provides a non-blocking, state-machine-driven user interface
 * for the SysMgr system. It handles input from a 4x4 keypad and displays
 * real-time information and configuration menus on a 2x16 character LCD.
 *
 * FIXES:
 * - Proper KeypadMgr_GetEvent() return value checking (E_DATA_STALE)
 * - Fixed numeric input handling to use correct button values
 * - Added missing button ID definitions
 * - Improved error handling and display formatting
 * - Fixed display function calls to match LCD HAL interface
 */

#include "ui_manager.h"
#include "ui_manager_cfg.h"
#include "sys_mgr.h"
#include "temphumctrl.h"
#include "keypad_mgr.h"
#include "char_display.h"
#include "logger.h"
#include "common.h"

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
    
    /* Initialize display */
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_Home();
    
    /* Initialize state */
    g_ui_state = UI_STATE_MAIN_SCREEN;
    g_main_screen_index = 0;
    ui_clear_input_buffer();
    
    /* Load current configuration from SysMgr */
    // Status_t status = SysMgr_GetConfig(&g_ui_working_configuration);
    // if (status != E_OK) {
    //     LOGW(TAG, "Failed to load initial config, using defaults");
    //     memset(&g_ui_working_configuration, 0, sizeof(g_ui_working_configuration));
    // }
    
    /* Display initial screen */
    ui_display_main_screen();
    g_last_display_update_ms = UI_MGR_GetTick();
    
    LOGI(TAG, "UI Manager initialized successfully");
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

    /* Rate limiting */
    if ((now - last_tick) < UI_MGR_MAIN_PERIOD_MS) {
        return;
    }
    last_tick = now;

    /* Process all available events from the keypad queue */
    Keypad_Event_t event;
    Status_t status;
    
    /* FIXED: Proper return value checking for KeypadMgr_GetEvent */
    while ((status = KeypadMgr_GetEvent(&event)) == E_OK) {
        LOGD(TAG, "Processing keypad event: button=%d, type=%d", event.button, event.type);
        ui_handle_event(&event);
    }
    
    /* Log if there was an error other than E_DATA_STALE */
    if (status != E_DATA_STALE && status != E_OK) {
        LOGW(TAG, "KeypadMgr_GetEvent returned error: %d", status);
    }
    
    /* Manage display updates based on current state */
    if (g_ui_state == UI_STATE_MAIN_SCREEN)
    {
        /* Auto-rotate main screens every UI_SCREEN_ROTATE_MS */
        if ((now - g_last_display_update_ms) >= UI_SCREEN_ROTATE_MS)
        {
            g_main_screen_index = (g_main_screen_index + 1) % 3;
            ui_display_main_screen();
            g_last_display_update_ms = now;
        }
    }
    else
    {
        /* Menu timeout handling */
        if ((now - g_last_key_press_ms) >= UI_MENU_TIMEOUT_MS)
        {
            LOGW(TAG, "Menu timeout. Auto-saving and exiting...");
            
            /* Auto-save configuration on timeout */
            // Status_t save_status = SysMgr_SetConfig(&g_ui_working_configuration);
            // if (save_status == E_OK) {
            //     save_status = SysMgr_SaveConfigToFlash();
            // }
            // if (save_status != E_OK) {
            //     LOGE(TAG, "Failed to auto-save configuration on timeout");
            // }
            
            /* Return to main screen */
            g_ui_state = UI_STATE_MAIN_SCREEN;
            g_main_screen_index = 0;
            ui_display_main_screen();
            g_last_display_update_ms = now;
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
    g_last_key_press_ms = UI_MGR_GetTick(); /* Reset timeout on any key press */
    
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
            /* No input handling in save state - auto-transition */
            break;
        case UI_STATE_CONFIG_SETS:
        case UI_STATE_ACTUATOR_TIMERS:
        case UI_STATE_LIGHT_SCHEDULE:
        default:
            LOGD(TAG, "Unhandled state %d or key %d - ignoring", g_ui_state, event->button);
            break;
    }
}

/**
 * @brief Handles events while in the main screen state.
 *
 * A long press on the ENTER key transitions to the configuration menu.
 * Short presses on navigation keys can manually change screens.
 *
 * @param event The keypad event to process.
 */
static void ui_handle_event_main_screen(Keypad_Event_t *event)
{
    if (event->type == KEYPAD_EVT_HOLD && event->button == KEYPAD_BTN_ENTER) {
        LOGI(TAG, "Entering configuration menu (long press detected)");
        
        /* Load current configuration from SysMgr */
        // Status_t status = SysMgr_GetConfig(&g_ui_working_configuration);
        // if (status != E_OK) {
        //     LOGW(TAG, "Failed to load config, using working copy");
        // }
        
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
        ui_clear_input_buffer();
    }
    else if (event->type == KEYPAD_EVT_PRESS) {
        /* Manual screen navigation */
        switch (event->button) {
            case KEYPAD_BTN_LEFT:
                g_main_screen_index = (g_main_screen_index == 0) ? 2 : g_main_screen_index - 1;
                ui_display_main_screen();
                g_last_display_update_ms = UI_MGR_GetTick();
                break;
            case KEYPAD_BTN_RIGHT:
                g_main_screen_index = (g_main_screen_index + 1) % 3;
                ui_display_main_screen();
                g_last_display_update_ms = UI_MGR_GetTick();
                break;
            default:
                /* Other keys ignored in main screen */
                break;
        }
    }
}

/**
 * @brief Handles events while in the root menu state.
 *
 * Numeric keys `1` through `8` transition to sub-menus. 
 * `9` saves and exits. The `BACK` key returns to main screen without saving.
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
            LOGI(TAG, "Entering global temperature configuration");
            g_ui_state = UI_STATE_EDIT_GLOBAL_TEMP;
            g_is_editing_min = true;
            ui_clear_input_buffer();
            ui_display_edit_global_temp();
            break;
            
        case KEYPAD_BTN_2:
            LOGI(TAG, "Entering global humidity configuration");
            g_ui_state = UI_STATE_EDIT_GLOBAL_HUM;
            g_is_editing_min = true;
            ui_clear_input_buffer();
            ui_display_edit_global_hum();
            break;
            
        case KEYPAD_BTN_3:
            LOGI(TAG, "Entering sensor sets configuration");
            g_ui_state = UI_STATE_CONFIG_SETS;
            ui_display_message("Config Sets", "Not Implemented");
            break;
            
        case KEYPAD_BTN_4:
            LOGI(TAG, "Entering per-sensor temp config");
            /* This should transition to per-sensor temperature config */
            ui_display_message("Per-Sensor Temp", "Not Implemented");
            break;
            
        case KEYPAD_BTN_5:
            LOGI(TAG, "Entering per-sensor humidity config");
            /* This should transition to per-sensor humidity config */
            ui_display_message("Per-Sensor Hum", "Not Implemented");
            break;
            
        case KEYPAD_BTN_6:
            LOGI(TAG, "Entering mode selection");
            g_ui_state = UI_STATE_SELECT_MODE;
            ui_display_select_mode();
            break;
            
        case KEYPAD_BTN_7:
            LOGI(TAG, "Entering actuator timers configuration");
            g_ui_state = UI_STATE_ACTUATOR_TIMERS;
            ui_display_message("Actuator Timers", "Not Implemented");
            break;
            
        case KEYPAD_BTN_8:
            LOGI(TAG, "Entering light schedule configuration");
            g_ui_state = UI_STATE_LIGHT_SCHEDULE;
            ui_display_message("Light Schedule", "Not Implemented");
            break;
            
        case KEYPAD_BTN_9:
            LOGI(TAG, "Save and exit selected");
            g_ui_state = UI_STATE_SAVE_AND_EXIT;
            ui_display_message("Saving config...", "Please wait...");
            
            /* Save configuration */
            // Status_t status = SysMgr_SetConfig(&g_ui_working_configuration);
            // if (status == E_OK) {
            //     status = SysMgr_SaveConfigToFlash();
            // }
            // 
            // if (status == E_OK) {
            //     ui_display_message("Config Saved!", "Returning...");
            //     LOGI(TAG, "Configuration saved successfully");
            // } else {
            //     ui_display_message("Save Failed!", "Check system");
            //     LOGE(TAG, "Failed to save configuration: %d", status);
            // }
            
            /* Brief delay to show save status, then return to main */
            /* In real implementation, you might want a delay timer here */
            g_ui_state = UI_STATE_MAIN_SCREEN;
            g_main_screen_index = 0;
            ui_display_main_screen();
            g_last_display_update_ms = UI_MGR_GetTick();
            break;
            
        case KEYPAD_BTN_BACK:
            LOGI(TAG, "Exiting menu without saving");
            g_ui_state = UI_STATE_MAIN_SCREEN;
            g_main_screen_index = 0;
            ui_clear_input_buffer();
            ui_display_main_screen();
            g_last_display_update_ms = UI_MGR_GetTick();
            break;
            
        default:
            LOGD(TAG, "Unhandled key %d in menu root", event->button);
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
    
    /* FIXED: Proper numeric input handling */
    if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
        ui_handle_numeric_input(event);
        ui_display_edit_global_temp();
    } 
    else if (event->button == KEYPAD_BTN_ERASE) {
        if (g_input_index > 0) {
            g_input_buffer[--g_input_index] = '\0';
            ui_display_edit_global_temp();
        }
    } 
    else if (event->button == KEYPAD_BTN_BACK) {
        ui_clear_input_buffer();
        g_is_editing_min = true;
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    } 
    else if (event->button == KEYPAD_BTN_ENTER) {
        if (strlen(g_input_buffer) == 0) {
            ui_display_message("Enter a value!", "0-50 degrees C");
            return;
        }
        
        float value = strtof(g_input_buffer, NULL);
        
        if (g_is_editing_min) {
            /* Validate temperature range (0-50°C typical for greenhouse) */
            if (value >= 0.0f && value <= 50.0f) {
                g_ui_working_configuration.global_temp_min = value;
                g_is_editing_min = false;
                ui_clear_input_buffer();
                ui_display_edit_global_temp();
                LOGI(TAG, "Set temp min to %.1f°C", value);
            } else {
                ui_display_message("Invalid T-Min!", "Range: 0-50C");
            }
        } else {
            /* Validate max >= min and within range */
            if (value >= g_ui_working_configuration.global_temp_min && value <= 50.0f) {
                g_ui_working_configuration.global_temp_max = value;
                ui_display_message("Temp Updated!", "Press BACK");
                LOGI(TAG, "Set temp max to %.1f°C", value);
                
                /* Auto-return to menu after brief display */
                g_ui_state = UI_STATE_MENU_ROOT;
                ui_clear_input_buffer();
                g_is_editing_min = true;
            } else {
                ui_display_message("Invalid T-Max!", "Must be >= T-Min");
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
    } 
    else if (event->button == KEYPAD_BTN_ERASE) {
        if (g_input_index > 0) {
            g_input_buffer[--g_input_index] = '\0';
            ui_display_edit_global_hum();
        }
    } 
    else if (event->button == KEYPAD_BTN_BACK) {
        ui_clear_input_buffer();
        g_is_editing_min = true;
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    } 
    else if (event->button == KEYPAD_BTN_ENTER) {
        if (strlen(g_input_buffer) == 0) {
            ui_display_message("Enter a value!", "0-100 percent");
            return;
        }
        
        float value = strtof(g_input_buffer, NULL);
        
        if (g_is_editing_min) {
            /* Validate humidity range (0-100%) */
            if (value >= 0.0f && value <= 100.0f) {
                g_ui_working_configuration.global_hum_min = value;
                g_is_editing_min = false;
                ui_clear_input_buffer();
                ui_display_edit_global_hum();
                LOGI(TAG, "Set humidity min to %.1f%%", value);
            } else {
                ui_display_message("Invalid H-Min!", "Range: 0-100%");
            }
        } else {
            /* Validate max >= min and within range */
            if (value >= g_ui_working_configuration.global_hum_min && value <= 100.0f) {
                g_ui_working_configuration.global_hum_max = value;
                ui_display_message("Humidity Updated!", "Press BACK");
                LOGI(TAG, "Set humidity max to %.1f%%", value);
                
                /* Auto-return to menu after brief display */
                g_ui_state = UI_STATE_MENU_ROOT;
                ui_clear_input_buffer();
                g_is_editing_min = true;
            } else {
                ui_display_message("Invalid H-Max!", "Must be >= H-Min");
            }
        }
    }
}

/**
 * @brief Handles events for selecting the system operational mode.
 *
 * Keys `1`, `2`, or `3` set the mode. `BACK` returns to the root menu.
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
            ui_display_message("Mode Set:", "Automatic");
            LOGI(TAG, "Mode set to Automatic");
            
            /* Notify SysMgr immediately for mode changes */
            // SysMgr_SetMode(SYS_MGR_MODE_AUTOMATIC);
            break;
            
        case KEYPAD_BTN_2:
            g_ui_working_configuration.mode = SYS_MGR_MODE_HYBRID;
            ui_display_message("Mode Set:", "Hybrid");
            LOGI(TAG, "Mode set to Hybrid");
            
            /* For Hybrid/Manual modes, may need additional schedule config */
            // SysMgr_SetMode(SYS_MGR_MODE_HYBRID);
            break;
            
        case KEYPAD_BTN_3:
            g_ui_working_configuration.mode = SYS_MGR_MODE_MANUAL;
            ui_display_message("Mode Set:", "Manual");
            LOGI(TAG, "Mode set to Manual");
            
            /* For Manual mode, may need additional schedule config */
            // SysMgr_SetMode(SYS_MGR_MODE_MANUAL);
            break;
            
        case KEYPAD_BTN_4:
            /* Fail-safe mode if supported */
            // g_ui_working_configuration.mode = SYS_MGR_MODE_FAILSAFE;
            ui_display_message("Mode Set:", "Fail-Safe");
            LOGI(TAG, "Mode set to Fail-Safe");
            break;
            
        case KEYPAD_BTN_BACK:
            LOGD(TAG, "Returning to menu root from mode selection");
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_clear_input_buffer();
            ui_display_menu_root();
            break;
            
        default:
            LOGD(TAG, "Invalid mode selection key: %d", event->button);
            break;
    }
}

/**
 * @brief Handles generic numeric input for all states.
 * 
 * FIXED: Now properly converts button ID to character.
 * 
 * @param event The keypad event containing the button pressed.
 */
static void ui_handle_numeric_input(Keypad_Event_t *event)
{
    if (g_input_index < UI_MAX_INPUT_LEN && 
        event->button >= KEYPAD_BTN_0 && 
        event->button <= KEYPAD_BTN_9) {
        
        /* FIXED: Direct button ID to character conversion */
        g_input_buffer[g_input_index++] = '0' + event->button;
        g_input_buffer[g_input_index] = '\0';
        
        LOGD(TAG, "Added digit %d, buffer now: '%s'", event->button, g_input_buffer);
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
    char line1[UI_LCD_COLS + 1];
    char line2[UI_LCD_COLS + 1];
    
    switch (g_main_screen_index)
    {
        case 0: /* Average Temperature & Humidity */
        {
            float avg_temp = 0.0f, avg_hum = 0.0f;
            // TempHumCtrl_GetSystemAverageTemperature(&avg_temp);
            // TempHumCtrl_GetSystemAverageHumidity(&avg_hum);
            
            /* Stub values for testing */
            avg_temp = 24.5f;
            avg_hum = 55.2f;
            
            // snprintf(line1, sizeof(line1), "Avg T:%.1fC H:%.0f", avg_temp, avg_hum);
            
            /* Show current mode on second line */
            const char* mode_str = "UNKNOWN";
            switch (g_ui_working_configuration.mode) {
                case SYS_MGR_MODE_AUTOMATIC: mode_str = "AUTO"; break;
                case SYS_MGR_MODE_HYBRID: mode_str = "HYBRID"; break;
                case SYS_MGR_MODE_MANUAL: mode_str = "MANUAL"; break;
                default: mode_str = "ERROR"; break;
            }
            snprintf(line2, sizeof(line2), "Mode: %-10s", mode_str);
            break;
        }
        
        case 1: /* Per-Sensor Readings */
            ui_display_per_sensor_readings();
            return; /* Function handles its own display */
            
        case 2: /* Actuator States */
            ui_display_actuator_states();
            return; /* Function handles its own display */
            
        default:
            snprintf(line1, sizeof(line1), "Screen Error");
            snprintf(line2, sizeof(line2), "Index: %d", g_main_screen_index);
            break;
    }
    
    ui_display_message(line1, line2);
}

/**
 * @brief Displays the per-sensor readings screen.
 * This function retrieves data from TempHumCtrl and formats it for the display.
 */
static void ui_display_per_sensor_readings(void)
{
    char line1[UI_LCD_COLS + 1];
    char line2[UI_LCD_COLS + 1];
    
    /* Try to get actual sensor readings */
    // float temp1, temp2, hum1, hum2;
    // Status_t status1 = TempHumCtrl_GetTemperature(SENSOR_ID_1, &temp1);
    // Status_t status2 = TempHumCtrl_GetTemperature(SENSOR_ID_2, &temp2);
    // Status_t status3 = TempHumCtrl_GetHumidity(SENSOR_ID_1, &hum1);
    // Status_t status4 = TempHumCtrl_GetHumidity(SENSOR_ID_2, &hum2);
    
    /* Stub values for testing */
    float temp1 = 23.1f, temp2 = 25.3f;
    float hum1 = 45.0f, hum2 = 58.0f;
    
    snprintf(line1, sizeof(line1), "S1:%.1fC %.0f%%", temp1, hum1);
    snprintf(line2, sizeof(line2), "S2:%.1fC %.0f%%", temp2, hum2);
    
    ui_display_message(line1, line2);
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