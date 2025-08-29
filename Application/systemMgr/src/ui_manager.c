/**
 * @file ui_manager.c
 * @brief User Interface Manager (LCD + Keypad)
 * @version 2.0
 * @date 2025
 *
 * This component provides a non-blocking, state-machine-driven user interface
 * for the SysMgr system. It handles input from a 4x4 keypad and displays
 * real-time information and configuration menus on a 2x16 character LCD.
 *
 * The core responsibilities include:
 * - Rotating through main monitoring screens.
 * - Navigating a multi-level configuration menu.
 * - Handling user input for numeric data and commands.
 * - Committing and saving user configurations to the SysMgr module.
 * - Implementing a timeout mechanism for auto-saving and exiting the menu.
 */

#include "ui_manager.h"
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

#define UI_SCREEN_ROTATE_MS     3000   /**< Time to rotate main screens (ms) */
#define UI_MENU_TIMEOUT_MS      60000  /**< Inactivity timeout for menu (ms) */
#define UI_MAX_INPUT_LEN        8      /**< Max characters for numeric input */



/* =============================================================================
 * PRIVATE GLOBAL VARIABLES
 * ============================================================================= */

static UI_State_t g_ui_state;
static uint32_t g_ui_screen_rotation_timer_ms;
static uint32_t g_ui_inactivity_timer_ms;
static uint8_t g_ui_screen_idx;
static SysMgr_Config_t g_ui_working_configuration;
static char g_ui_input_buffer[UI_MAX_INPUT_LEN];
static uint8_t g_ui_input_buffer_length;

static const char *TAG = "TempHumCtrl";
/* =============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================================= */
static void ui_clear_input_buffer(void);
static void ui_append_digit(char digit);
static bool ui_fetch_key(char *key_out);

/* --- Display Functions --- */
static void ui_display_main_screen(void);
static void ui_display_menu_root(void);
static void ui_display_edit_global_temp(void);
static void ui_display_edit_global_hum(void);
static void ui_display_select_mode(void);
static void ui_display_message(const char *line1, const char *line2);

/* --- State Transition Functions --- */
static void ui_enter_edit_global_temp(void);
static void ui_enter_edit_global_hum(void);
static void ui_enter_select_mode(void);
static void ui_commit_and_save_configuration(void);

/* --- Handler Functions --- */
static void ui_handle_input_main_screen(char key);
static void ui_handle_input_menu_root(char key);
static void ui_handle_input_edit_global_temp(char key);
static void ui_handle_input_edit_global_hum(char key);
static void ui_handle_input_select_mode(char key);

/* =============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================= */

/**
 * @brief Initializes the UI Manager component.
 *
 * Sets up initial state, retrieves the current system configuration, and
 * displays the first main screen.
 *
 * @return Status_t E_OK on success.
 */
Status_t UIMgr_Init(void)
{
    g_ui_state = UI_STATE_MAIN_SCREEN;
    g_ui_screen_rotation_timer_ms = 0;
    g_ui_inactivity_timer_ms = 0;
    g_ui_screen_idx = 0;
    ui_clear_input_buffer();

    SYS_MGR_GetConfig(&g_ui_working_configuration);
    ui_display_main_screen();
    LOGI(TAG,"UI Manager initialized.");

    return E_OK;
}

/**
 * @brief Main periodic function for the UI Manager.
 *
 * This function should be called frequently to handle screen rotation,
 * keypad input, and menu timeouts.
 *
 * @param tick_ms Time in milliseconds since the last call.
 */
void UIMgr_MainFunction()
{
    g_ui_inactivity_timer_ms += UI_MGR_MAIN_PERIOD_MS;
    char key_pressed;
    bool key_event = ui_fetch_key(&key_pressed);
    if (key_event) 
    {
        g_ui_inactivity_timer_ms = 0; // Reset timer on any key press
    }

    /* Handle menu timeout */
    if (g_ui_state != UI_STATE_MAIN_SCREEN && g_ui_inactivity_timer_ms >= UI_MENU_TIMEOUT_MS) 
    {
        ui_display_message("Auto-saving &", "Exiting...");
        SYS_MGR_UpdateConfigRuntime(&g_ui_working_configuration);
        g_ui_state = UI_STATE_MAIN_SCREEN;
        g_ui_inactivity_timer_ms = 0;
        g_ui_screen_rotation_timer_ms = 0;
        return; // Exit early to prevent further processing this cycle
    }

    /* State Machine Logic */
    switch (g_ui_state) 
    {
        case UI_STATE_MAIN_SCREEN:
            if (key_event) 
            {
                ui_handle_input_main_screen(key_pressed);
            }
            // Screen rotation logic
            g_ui_screen_rotation_timer_ms += UI_MGR_MAIN_PERIOD_MS;
            if (g_ui_screen_rotation_timer_ms >= UI_SCREEN_ROTATE_MS) 
            {
                g_ui_screen_idx = (g_ui_screen_idx + 1) % 3; // Cycle through 3 screens
                ui_display_main_screen();
                g_ui_screen_rotation_timer_ms = 0;
            }
            break;

        case UI_STATE_MENU_ROOT:
            if (key_event) {
                ui_handle_input_menu_root(key_pressed);
            }
            break;

        case UI_STATE_EDIT_GLOBAL_TEMP:
            if (key_event) {
                ui_handle_input_edit_global_temp(key_pressed);
            }
            break;

        case UI_STATE_EDIT_GLOBAL_HUM:
            if (key_event) {
                ui_handle_input_edit_global_hum(key_pressed);
            }
            break;
        
        case UI_STATE_SELECT_MODE:
            if (key_event) {
                ui_handle_input_select_mode(key_pressed);
            }
            break;

        case UI_STATE_SAVE_AND_EXIT:
            ui_commit_and_save_configuration();
            g_ui_state = UI_STATE_MAIN_SCREEN;
            ui_display_main_screen(); // Show first main screen after saving
            break;

        default:
            LOGW(TAG,"UI Manager in unhandled state: %d", g_ui_state);
            g_ui_state = UI_STATE_MAIN_SCREEN;
            ui_display_main_screen();
            break;
    }
}

/* =============================================================================
 * PRIVATE UTILITY FUNCTIONS
 * ============================================================================= */

/**
 * @brief Clears the numeric input buffer.
 */
static void ui_clear_input_buffer(void)
{
    g_ui_input_buffer_length = 0;
    memset(g_ui_input_buffer, '\0', sizeof(g_ui_input_buffer));
}

/**
 * @brief Appends a digit to the input buffer.
 * @param digit The character digit to append.
 */
static void ui_append_digit(char digit)
{
    if (g_ui_input_buffer_length < UI_MAX_INPUT_LEN - 1) {
        g_ui_input_buffer[g_ui_input_buffer_length++] = digit;
        g_ui_input_buffer[g_ui_input_buffer_length] = '\0';
    }
}

/**
 * @brief Fetches a single key event from the Keypad Manager.
 * @param key_out Pointer to store the key character.
 * @return true if a key was pressed, false otherwise.
 */
static bool ui_fetch_key(char *key_out)
{
    Keypad_Event_t event;
    if (KeypadMgr_GetLastEvent(&event) == E_OK) {
        *key_out = event.event_type;
        return true;
    }
    return false;
}

/* =============================================================================
 * PRIVATE DISPLAY FUNCTIONS
 * ============================================================================= */

/**
 * @brief Displays a message on the LCD.
 * @param line1 The string for the first line.
 * @param line2 The string for the second line.
 */
static void ui_display_message(const char *line1, const char *line2)
{
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_Home();
    HAL_CharDisplay_WriteString(line1);
    HAL_CharDisplay_SetCursor(1, 0);
    HAL_CharDisplay_WriteString(line2);
}

/**
 * @brief Displays one of the main rotating screens.
 */
static void ui_display_main_screen(void)
{
    Status_t s_temp = E_OK;
    Status_t s_hum = E_OK;
    char line1[17], line2[17];
    float temp, hum;
    float rdg;
    
    switch (g_ui_screen_idx) {
        case 0:
            s_temp = TempHumCtrl_GetSystemAverageTemperature(&temp);
            s_hum = TempHumCtrl_GetSystemAverageHumidity(&hum);
            // Average Temp & Hum screen
            if ((s_temp == E_OK) && (s_hum == E_OK)) 
            {
                snprintf(line1, sizeof(line1), "Avg T:%.1fC H:%.0f%%", temp, hum);
                snprintf(line2, sizeof(line2), "Mode: %d", g_ui_working_configuration.mode);
            } 
            else 
            {
                snprintf(line1, sizeof(line1), "Avg T:-- H:--%%");
                snprintf(line2, sizeof(line2), "Mode: %d", g_ui_working_configuration.mode);
            }
            break;
        case 1:
            // Per-sensor Temp & Hum screen (shows first two sensors)
            snprintf(line1, sizeof(line1), "S1:--C --%%");
            snprintf(line2, sizeof(line2), "S2:--C --%%");

            s_temp = TempHumCtrl_GetAverageTemperature(TEMPHUM_SENSOR_ID_DHT_1, &rdg);
            
            if (s_temp == E_OK) 
            {
                snprintf(line1, sizeof(line1), "S1:%.1fC %.0f%%", rdg, rdg);
            }
            s_temp = TempHumCtrl_GetAverageTemperature(TEMPHUM_SENSOR_ID_DHT_2, &rdg);
            if (s_temp == E_OK) 
            {
                snprintf(line2, sizeof(line2), "S2:%.1fC %.0f%%", rdg, rdg);
            }
            break;
        case 2:
            // Actuator Status screen
            // NOTE: This will require a new API in sys_mgr.h
            snprintf(line1, sizeof(line1), "Fan:OFF Pump:OFF");
            snprintf(line2, sizeof(line2), "Htr:OFF Vent:OFF");
            break;
    }
    ui_display_message(line1, line2);
}

/**
 * @brief Displays the main menu.
 */
static void ui_display_menu_root(void)
{
    ui_display_message("1:Global T  2:Global H", "3:Sets Cfg 4:Mode");
}

/**
 * @brief Displays the global temperature edit screen.
 */
static void ui_display_edit_global_temp(void)
{
    ui_display_message("Set Global Temp", g_ui_input_buffer);
}

/**
 * @brief Displays the global humidity edit screen.
 */
static void ui_display_edit_global_hum(void)
{
    ui_display_message("Set Global Humidity", g_ui_input_buffer);
}

/**
 * @brief Displays the mode selection screen.
 */
static void ui_display_select_mode(void)
{
    const char* mode_name;
    switch(g_ui_working_configuration.mode) {
        case SYS_MGR_MODE_AUTOMATIC: mode_name = "Automatic"; break;
        case SYS_MGR_MODE_HYBRID:    mode_name = "Hybrid"; break;
        case SYS_MGR_MODE_MANUAL:    mode_name = "Manual"; break;
        default: mode_name = "Unknown"; break;
    }
    char line2[17];
    // snprintf(line2, sizeof(line2), "Current: %s", mode_name);
    // ui_display_message("Select Mode (1-3)", line2);
}

/* =============================================================================
 * PRIVATE STATE TRANSITION FUNCTIONS
 * ============================================================================= */

/**
 * @brief Transitions to the global temperature edit state.
 */
static void ui_enter_edit_global_temp(void)
{
    g_ui_state = UI_STATE_EDIT_GLOBAL_TEMP;
    ui_clear_input_buffer();
    char temp_str[10];
    snprintf(temp_str, sizeof(temp_str), "%.1f %.1f", g_ui_working_configuration.global_temp_min, g_ui_working_configuration.global_temp_max);
    ui_display_message("Edit T (min max):", temp_str);
}

/**
 * @brief Transitions to the global humidity edit state.
 */
static void ui_enter_edit_global_hum(void)
{
    g_ui_state = UI_STATE_EDIT_GLOBAL_HUM;
    ui_clear_input_buffer();
    char hum_str[10];
    snprintf(hum_str, sizeof(hum_str), "%.1f %.1f", g_ui_working_configuration.global_hum_min, g_ui_working_configuration.global_hum_max);
    ui_display_message("Edit H (min max):", hum_str);
}

/**
 * @brief Transitions to the mode selection state.
 */
static void ui_enter_select_mode(void)
{
    g_ui_state = UI_STATE_SELECT_MODE;
    ui_display_select_mode();
}

/**
 * @brief Commits the working configuration to SysMgr and saves it.
 */
static void ui_commit_and_save_configuration(void)
{
    if (SYS_MGR_UpdateConfigRuntime(&g_ui_working_configuration) == E_OK) {
        ui_display_message("Config Saved!", "Exiting Menu...");
    } else {
        ui_display_message("Save Failed!", "Exiting Menu...");
        // Report fault to System Monitor here
    }
}

/* =============================================================================
 * PRIVATE INPUT HANDLER FUNCTIONS
 * ============================================================================= */

/**
 * @brief Handles keypad input while in the main screen state.
 * @param key The key character pressed.
 */
static void ui_handle_input_main_screen(char key)
{
    if (key == '#') {
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
        SYS_MGR_GetConfig(&g_ui_working_configuration); // Get fresh config for editing
    }
}

/**
 * @brief Handles keypad input while in the menu root state.
 * @param key The key character pressed.
 */
static void ui_handle_input_menu_root(char key)
{
    switch (key) {
        case '1': ui_enter_edit_global_temp(); break;
        case '2': ui_enter_edit_global_hum(); break;
        case '3': ui_display_message("Sets Config", "Not Implemented!"); break;
        case '4': ui_enter_select_mode(); break;
        case '9': g_ui_state = UI_STATE_SAVE_AND_EXIT; break;
        case '*': g_ui_state = UI_STATE_MAIN_SCREEN; ui_display_main_screen(); break;
        default: ui_display_message("Invalid Option!", "Press * to go back"); break;
    }
}

/**
 * @brief Handles keypad input while editing global temperature.
 * @param key The key character pressed.
 */
static void ui_handle_input_edit_global_temp(char key)
{
    if (isdigit(key) || key == '.') {
        ui_append_digit(key);
        ui_display_edit_global_temp();
    } else if (key == '#') {
        // Parse and validate the input
        float min_temp, max_temp;
        if (sscanf(g_ui_input_buffer, "%f %f", &min_temp, &max_temp) == 2 &&
            min_temp >= 0.0f && min_temp <= 99.0f &&
            max_temp >= 0.0f && max_temp <= 99.0f &&
            min_temp < max_temp) {
            
            g_ui_working_configuration.global_temp_min = min_temp;
            g_ui_working_configuration.global_temp_max = max_temp;
            ui_display_message("T-min/max Updated!", "Press * to go back");
        } else {
            ui_display_message("Invalid Input!", "T Range: 0-99C");
        }
        ui_clear_input_buffer();
        g_ui_state = UI_STATE_MENU_ROOT;
    } else if (key == '*') {
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_clear_input_buffer();
        ui_display_menu_root();
    }
}

/**
 * @brief Handles keypad input while editing global humidity.
 * @param key The key character pressed.
 */
static void ui_handle_input_edit_global_hum(char key)
{
    if (isdigit(key) || key == '.') {
        ui_append_digit(key);
        ui_display_edit_global_hum();
    } else if (key == '#') {
        float min_hum, max_hum;
        if (sscanf(g_ui_input_buffer, "%f %f", &min_hum, &max_hum) == 2 &&
            min_hum >= 0.0f && min_hum <= 100.0f &&
            max_hum >= 0.0f && max_hum <= 100.0f &&
            min_hum < max_hum) {

            g_ui_working_configuration.global_hum_min = min_hum;
            g_ui_working_configuration.global_hum_max = max_hum;
            ui_display_message("H-min/max Updated!", "Press * to go back");
        } else {
            ui_display_message("Invalid Input!", "H Range: 0-100%");
        }
        ui_clear_input_buffer();
        g_ui_state = UI_STATE_MENU_ROOT;
    } else if (key == '*') {
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_clear_input_buffer();
        ui_display_menu_root();
    }
}

/**
 * @brief Handles keypad input while selecting the mode.
 * @param key The key character pressed.
 */
static void ui_handle_input_select_mode(char key)
{
    switch (key) {
        case '1':
            g_ui_working_configuration.mode = SYS_MGR_MODE_AUTOMATIC;
            ui_display_message("Mode Set", "Automatic");
            break;
        case '2':
            g_ui_working_configuration.mode = SYS_MGR_MODE_HYBRID;
            ui_display_message("Mode Set", "Hybrid");
            break;
        case '3':
            g_ui_working_configuration.mode = SYS_MGR_MODE_MANUAL;
            ui_display_message("Mode Set", "Manual");
            break;
        case '*':
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
            break;
        default:
            ui_display_message("Invalid Option!", "Select 1-3");
            break;
    }
    // Return to menu after a short delay or on next key press
}