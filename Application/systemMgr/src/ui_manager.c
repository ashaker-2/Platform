/**
 * @file ui_manager.c
 * @brief User Interface Manager (complete state-machine-driven implementation)
 * @version 3.0
 * @date 2025
 *
 * Full implementation of the UI Manager as a state machine.
 * - Supports dashboard rotation (System Average + Actuator states)
 * - Supports configuration menu (Temp, Hum, PerSens placeholder, Fan, Vent, Pump, Heater, Light)
 * - Consistent key behavior:
 *   - ENTER short = confirm/save
 *   - ENTER hold  = special (toggle unit when editing actuator times) or menu entry when on main screens
 *   - BACK short  = erase last digit
 *   - BACK hold   = cancel & return to menu
 *   - Arrow keys  = field navigation; hold LEFT/RIGHT to switch actuators in actuator edit
 *
 * The file is intentionally heavily commented to make it easy to modify and extend.
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
#include <stdbool.h>

/* ---------------------------------------------------------------------------
 * NOTE: You must map these keypad button IDs to your platform's definitions.
 * If your button enums are named differently, replace them here or include
 * a header that defines them. These are placeholders used in this implementation.
 * --------------------------------------------------------------------------- */
#ifndef KEYPAD_BTN_0
#define KEYPAD_BTN_0      0
#define KEYPAD_BTN_1      1
#define KEYPAD_BTN_2      2
#define KEYPAD_BTN_3      3
#define KEYPAD_BTN_4      4
#define KEYPAD_BTN_5      5
#define KEYPAD_BTN_6      6
#define KEYPAD_BTN_7      7
#define KEYPAD_BTN_8      8
#define KEYPAD_BTN_9      9
#define KEYPAD_BTN_ENTER  10
#define KEYPAD_BTN_BACK   11
#define KEYPAD_BTN_LEFT   12
#define KEYPAD_BTN_RIGHT  13
#define KEYPAD_BTN_UP     14
#define KEYPAD_BTN_DOWN   15
#endif

/* Short aliases for logger */
static const char *TAG = "UI_MGR";

/* ---------------------------------------------------------------------------
 * UI State machine
 * --------------------------------------------------------------------------- */

/* Top-level UI states */
typedef enum {
    UI_STATE_MAIN_SCREEN = 0,      /* dashboard rotation screens */
    UI_STATE_MENU_ROOT,            /* root configuration menu (paged) */
    UI_STATE_MENU_TEMP_SELECT,     /* select sensor for temp config */
    UI_STATE_MENU_TEMP_EDIT,       /* edit temp (Tmin/Tmax) */
    UI_STATE_MENU_HUM_SELECT,      /* select sensor for humidity config */
    UI_STATE_MENU_HUM_EDIT,        /* edit humidity (Hmin/Hmax) */
    UI_STATE_MENU_PERSENS,         /* placeholder per-sensor menu (not implemented) */
    UI_STATE_MENU_ACT_SELECT_TYPE, /* select actuator type (fan/vent/pump/heater) */
    UI_STATE_MENU_ACT_SELECT_ID,   /* select specific actuator instance */
    UI_STATE_MENU_ACT_MODE,        /* choose Auto/Manual for actuator */
    UI_STATE_MENU_ACT_MANUAL_EDIT, /* edit ON/OFF times for actuator (manual) */
    UI_STATE_MENU_LIGHT_EDIT,      /* edit light schedule (ON/OFF times) */
    UI_STATE_SAVE_AND_EXIT,        /* saving state */
    UI_STATE_COUNT
} UI_State_t;

/* Cursor positions for dual-field screens */
typedef enum {
    FIELD_FIRST = 0,
    FIELD_SECOND
} UI_Field_t;

/* Units for actuator times */
typedef enum {
    TIME_UNIT_SECONDS = 0,
    TIME_UNIT_MINUTES,
    TIME_UNIT_HOURS,
    TIME_UNIT_COUNT
} TimeUnit_t;

static const char *TimeUnitStr[TIME_UNIT_COUNT] = { "s", "m", "h" };

/* ---------------------------------------------------------------------------
 * Internal state & working variables
 * --------------------------------------------------------------------------- */

static UI_State_t g_ui_state = UI_STATE_MAIN_SCREEN;

/* dashboard rotation */
static uint8_t g_main_screen_index = 0; /* 0..(NUM_DASH_SCREENS-1) */
static uint32_t g_last_display_update_ms = 0;
static uint32_t g_last_key_press_ms = 0;

/* working copy of SysMgr config used during menu editing */
static SysMgr_Config_t g_ui_working_config;

/* input buffer for numeric entry (shared across menu screens) */
static char g_input_buffer[UI_MAX_INPUT_LEN + 1];
static uint8_t g_input_index = 0;

/* cursor / field selection */
static UI_Field_t g_current_field = FIELD_FIRST;

/* actuator editing context */
typedef struct {
    SYS_MGR_Actuator_t type;     /* actuator type being edited (Fan, Heater, etc.) */
    uint8_t actuator_id;         /* actuator index within type (0..count-1) */
    TimeUnit_t unit_on;          /* unit for ON field */
    TimeUnit_t unit_off;         /* unit for OFF field */
    uint32_t on_value;           /* stored value in seconds (persisted) */
    uint32_t off_value;          /* stored value in seconds (persisted) */
} ActuatorEditContext_t;

static ActuatorEditContext_t g_act_edit_ctx;

/* light editing context */
typedef struct {
    uint8_t on_hour;
    uint8_t on_min;
    uint8_t off_hour;
    uint8_t off_min;
} LightEditContext_t;

static LightEditContext_t g_light_ctx;

/* sensor selection context */
static uint8_t g_selected_sensor_index = 0; /* 0-based sensor index */
static uint8_t g_menu_page = 0; /* menu page (for root menu pagination) */

/* small helper to keep previous displayed strings so we can avoid flicker */
static char g_last_line1[UI_LCD_COLS + 1];
static char g_last_line2[UI_LCD_COLS + 1];

/* ---------------------------------------------------------------------------
 * Forward declarations (screen renderers and handlers)
 * --------------------------------------------------------------------------- */

/* public API */
void UI_MGR_Init(void);
void UI_MGR_MainFunction(void);

/* helpers */
static void ui_clear_input_buffer(void);
static void ui_display_message_lines(const char *l1, const char *l2);
static void ui_display_message_printf(const char *fmt1, const char *fmt2, ...);

/* main screen handlers */
static void ui_display_dashboard(void);
static void ui_handle_dashboard_event(Keypad_Event_t *event);

/* menu handlers */
static void ui_display_menu_root(void);
static void ui_handle_menu_root_event(Keypad_Event_t *event);

/* temp/hum handlers */
static void ui_display_temp_select(void);
static void ui_handle_temp_select_event(Keypad_Event_t *event);
static void ui_display_temp_edit(void);
static void ui_handle_temp_edit_event(Keypad_Event_t *event);

static void ui_display_hum_select(void);
static void ui_handle_hum_select_event(Keypad_Event_t *event);
static void ui_display_hum_edit(void);
static void ui_handle_hum_edit_event(Keypad_Event_t *event);

/* actuator handlers */
static void ui_display_act_select_type(void);
static void ui_handle_act_select_type_event(Keypad_Event_t *event);
static void ui_display_act_select_id(void);
static void ui_handle_act_select_id_event(Keypad_Event_t *event);
static void ui_display_act_mode(void);
static void ui_handle_act_mode_event(Keypad_Event_t *event);
static void ui_display_act_manual_edit(void);
static void ui_handle_act_manual_edit_event(Keypad_Event_t *event);

/* light handlers */
static void ui_display_light_edit(void);
static void ui_handle_light_edit_event(Keypad_Event_t *event);

/* utility */
static bool ui_parse_input_to_uint32(uint32_t *out_val, TimeUnit_t unit, uint32_t max_allowed);
static void ui_format_seconds_by_unit(char *buf, size_t buf_sz, uint32_t seconds, TimeUnit_t unit);
static void ui_refresh_if_changed(const char *l1, const char *l2);
static void ui_copy_to_last_lines(const char *l1, const char *l2);

/* validation helpers */
static bool validate_temp_range(float min_c, float max_c);
static bool validate_hum_range(float min_p, float max_p);
static bool validate_light_times(uint8_t on_h, uint8_t on_m, uint8_t off_h, uint8_t off_m);

/* save helper */
static void ui_save_working_config(void);

/* ---------------------------------------------------------------------------
 * Implementation
 * --------------------------------------------------------------------------- */

void UI_MGR_Init(void)
{
    LOGI(TAG, "UI_MGR_Init()");
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_SetCursor(0, 0);
    HAL_CharDisplay_EnableCursor(false);

    /* default state */
    g_ui_state = UI_STATE_MAIN_SCREEN;
    g_main_screen_index = 0;
    g_last_display_update_ms = UI_MGR_GetTick();
    g_last_key_press_ms = UI_MGR_GetTick();
    g_menu_page = 0;

    /* clear buffers */
    ui_clear_input_buffer();
    memset(&g_ui_working_config, 0, sizeof(g_ui_working_config));
    memset(g_last_line1, 0, sizeof(g_last_line1));
    memset(g_last_line2, 0, sizeof(g_last_line2));

    /* load current config snapshot for potential menu use */
    if (SYS_MGR_GetConfig(&g_ui_working_config) != E_OK) {
        /* If not available, use defaults */
        LOGW(TAG, "SYS_MGR_GetConfig failed - using defaults");
        /* If you want to explicitly copy defaults: g_ui_working_config = g_default_system_configuration; */
    }

    /* initial dashboard render */
    ui_display_dashboard();
}

/* Main periodic function - non-blocking and intended to be called every UI_MGR_MAIN_PERIOD_MS ms */
void UI_MGR_MainFunction(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = UI_MGR_GetTick();

    /* rate limiting */
    if ((now - last_tick) < UI_MGR_MAIN_PERIOD_MS) {
        return;
    }
    last_tick = now;

    /* Process keypad events (drain queue) */
    Keypad_Event_t event;
    Status_t status;
    while ((status = KeypadMgr_GetEvent(&event)) == E_OK) {
        /* Update last key press time for menu timeout logic */
        g_last_key_press_ms = now;

        /* Route event to the current state handler */
        switch (g_ui_state) {
            case UI_STATE_MAIN_SCREEN:
                ui_handle_dashboard_event(&event);
                break;
            case UI_STATE_MENU_ROOT:
                ui_handle_menu_root_event(&event);
                break;
            case UI_STATE_MENU_TEMP_SELECT:
                ui_handle_temp_select_event(&event);
                break;
            case UI_STATE_MENU_TEMP_EDIT:
                ui_handle_temp_edit_event(&event);
                break;
            case UI_STATE_MENU_HUM_SELECT:
                ui_handle_hum_select_event(&event);
                break;
            case UI_STATE_MENU_HUM_EDIT:
                ui_handle_hum_edit_event(&event);
                break;
            case UI_STATE_MENU_PERSENS:
                /* placeholder: just show not implemented and go back on any key */
                ui_display_message_lines("Per-Sensor", "Not Implemented");
                if (event.type == KEYPAD_EVT_PRESS || event.type == KEYPAD_EVT_HOLD) {
                    g_ui_state = UI_STATE_MENU_ROOT;
                    ui_display_menu_root();
                }
                break;
            case UI_STATE_MENU_ACT_SELECT_TYPE:
                ui_handle_act_select_type_event(&event);
                break;
            case UI_STATE_MENU_ACT_SELECT_ID:
                ui_handle_act_select_id_event(&event);
                break;
            case UI_STATE_MENU_ACT_MODE:
                ui_handle_act_mode_event(&event);
                break;
            case UI_STATE_MENU_ACT_MANUAL_EDIT:
                ui_handle_act_manual_edit_event(&event);
                break;
            case UI_STATE_MENU_LIGHT_EDIT:
                ui_handle_light_edit_event(&event);
                break;
            case UI_STATE_SAVE_AND_EXIT:
                /* ignore input while saving */
                break;
            default:
                LOGW(TAG, "Unhandled UI state: %d", g_ui_state);
                break;
        }
    }

    /* If KeypadMgr_GetEvent returned error not E_DATA_STALE, log it */
    if (status != E_OK && status != E_DATA_STALE) {
        LOGW(TAG, "KeypadMgr_GetEvent returned %d", status);
    }

    /* Handle periodic tasks per state */
    if (g_ui_state == UI_STATE_MAIN_SCREEN) {
        /* auto-rotate between top-level dashboard screens */
        if ((now - g_last_display_update_ms) >= UI_SCREEN_ROTATE_MS) {
            g_main_screen_index++;
            /* We have 2 main dashboard screens in final design: average and actuators */
            g_main_screen_index %= 2; /* 0..1 */
            ui_display_dashboard();
            g_last_display_update_ms = now;
        }
    } else {
        /* Menu timeout handling: if idle for too long, cancel unsaved and return to main */
        if ((now - g_last_key_press_ms) >= UI_MENU_TIMEOUT_MS) {
            LOGW(TAG, "Menu timeout - discarding unsaved and returning to dashboard");
            /* reload last saved config to working config to discard unpersisted edits */
            SYS_MGR_GetConfig(&g_ui_working_config);
            g_ui_state = UI_STATE_MAIN_SCREEN;
            g_main_screen_index = 0;
            ui_display_dashboard();
            g_last_display_update_ms = now;
        }
    }
}

/* ---------------------------------------------------------------------------
 * Basic helpers for input buffer and display
 * --------------------------------------------------------------------------- */

static void ui_clear_input_buffer(void)
{
    memset(g_input_buffer, 0, sizeof(g_input_buffer));
    g_input_index = 0;
}

static void ui_display_message_lines(const char *l1, const char *l2)
{
    char buf1[UI_LCD_COLS + 1];
    char buf2[UI_LCD_COLS + 1];
    /* ensure safe copy and clipping */
    if (!l1) l1 = "";
    if (!l2) l2 = "";
    strncpy(buf1, l1, UI_LCD_COLS);
    buf1[UI_LCD_COLS] = '\0';
    strncpy(buf2, l2, UI_LCD_COLS);
    buf2[UI_LCD_COLS] = '\0';
    ui_refresh_if_changed(buf1, buf2);
}

#include <stdarg.h>
static void ui_display_message_printf(const char *fmt1, const char *fmt2, ...)
{
    char buf1[UI_LCD_COLS + 1] = {0};
    char buf2[UI_LCD_COLS + 1] = {0};
    va_list ap;
    va_start(ap, fmt2);
    if (fmt1) {
        vsnprintf(buf1, sizeof(buf1), fmt1, ap);
    }
    /* For second format we need a second va_list; extract remaining args manually not needed here since handshake
       uses only at most two simple formats - to keep code simple, caller will pass literal formats or use
       ui_display_message_lines for complex formatting. */
    va_end(ap);
    if (fmt2) {
        /* no varargs for second format - keep simple */
        strncpy(buf2, fmt2, sizeof(buf2)-1);
    }
    ui_refresh_if_changed(buf1, buf2);
}

/* Render to LCD only if content changed (reduce flicker) */
static void ui_refresh_if_changed(const char *l1, const char *l2)
{
    if (strncmp(g_last_line1, l1, UI_LCD_COLS) == 0 &&
        strncmp(g_last_line2, l2, UI_LCD_COLS) == 0) {
        /* no change */
        return;
    }
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_SetCursor(0, 0);
    HAL_CharDisplay_WriteString(l1);
    HAL_CharDisplay_SetCursor(0, 1);
    HAL_CharDisplay_WriteString(l2);
    ui_copy_to_last_lines(l1, l2);
}

static void ui_copy_to_last_lines(const char *l1, const char *l2)
{
    memset(g_last_line1, 0, sizeof(g_last_line1));
    memset(g_last_line2, 0, sizeof(g_last_line2));
    strncpy(g_last_line1, l1, UI_LCD_COLS);
    strncpy(g_last_line2, l2, UI_LCD_COLS);
}

/* ---------------------------------------------------------------------------
 * Dashboard (main screens)
 * --------------------------------------------------------------------------- */

static void ui_display_dashboard(void)
{
    /* two screens:
     * index 0: system avg + mode + time
     * index 1: actuator aggregate states
     */
    char line1[UI_LCD_COLS + 1] = {0};
    char line2[UI_LCD_COLS + 1] = {0};

    if (g_main_screen_index == 0) {
        /* System Averages */
        float avg_t = 0.0f, avg_h = 0.0f;
        Status_t stt = TempHumCtrl_GetSystemAverageTemperature(&avg_t);
        Status_t sth = TempHumCtrl_GetSystemAverageHumidity(&avg_h);

        /* If read fails, keep previous display (don't overwrite). We'll check and only update successful fields. */
        char tbuf[20] = {0};
        char hbuf[20] = {0};
        if (stt == E_OK) snprintf(tbuf, sizeof(tbuf), "AvgT:%.1fC", avg_t);
        else snprintf(tbuf, sizeof(tbuf), "AvgT:--.-C");

        if (sth == E_OK) snprintf(hbuf, sizeof(hbuf), "H:%.0f%%", avg_h);
        else snprintf(hbuf, sizeof(hbuf), "H:--%%");

        snprintf(line1, sizeof(line1), "%s %s", tbuf, hbuf);

        /* Mode + time on line 2 */
        SYS_MGR_Mode_t mode;
        if (SYS_MGR_GetMode(&mode) == E_OK) {
            const char *mode_str = "UNK";
            switch (mode) {
                case SYS_MGR_MODE_AUTOMATIC: mode_str = "AUTO"; break;
                case SYS_MGR_MODE_HYBRID:    mode_str = "HYBR"; break;
                case SYS_MGR_MODE_MANUAL:    mode_str = "MAN";  break;
                case SYS_MGR_MODE_FAILSAFE:  mode_str = "SAFE"; break;
                default: mode_str = "UNK"; break;
            }
            /* time */
            SysClock_Time_t t = SysMgr_GetCurrentTime();
            snprintf(line2, sizeof(line2), "Mode:%s %02u:%02u", mode_str, t.hour, t.minute);
        } else {
            snprintf(line2, sizeof(line2), "Mode:??? --:--");
        }
    } else {
        /* Actuator aggregate states */
        SYS_MGR_Actuator_States_t states;
        if (SYS_MGR_GetActuatorStates(&states) == E_OK) {
            /* Build lines */
            snprintf(line1, sizeof(line1), "Fan:%s Heat:%s",
                    states.fans_active ? "ON " : "OFF",
                    states.heaters_active ? "ON " : "OFF");
            snprintf(line2, sizeof(line2), "Pump:%s Vent:%s",
                    states.pumps_active ? "ON " : "OFF",
                    states.vents_active ? "ON " : "OFF");
        } else {
            snprintf(line1, sizeof(line1), "Actuators N/A");
            snprintf(line2, sizeof(line2), "Check system");
        }
    }

    ui_refresh_if_changed(line1, line2);
}

/* Handle events on dashboard screens */
static void ui_handle_dashboard_event(Keypad_Event_t *event)
{
    if (event->type == KEYPAD_EVT_PRESS) {
        switch (event->button) {
            case KEYPAD_BTN_LEFT:
                g_main_screen_index = (g_main_screen_index == 0) ? 1 : (g_main_screen_index - 1);
                ui_display_dashboard();
                break;
            case KEYPAD_BTN_RIGHT:
                g_main_screen_index = (g_main_screen_index + 1) % 2;
                ui_display_dashboard();
                break;
            case KEYPAD_BTN_ENTER:
                /* short press - no-op on dashboard (or could show details) */
                break;
            case KEYPAD_BTN_BACK:
                /* short press - no-op */
                break;
            default:
                /* other keys ignored */
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_ENTER) {
            /* Enter config menu */
            /* refresh working config snapshot */
            if (SYS_MGR_GetConfig(&g_ui_working_config) != E_OK) {
                LOGW(TAG, "Failed to load config for editing; using last snapshot");
            }
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
            ui_clear_input_buffer();
        } else if (event->button == KEYPAD_BTN_BACK) {
            /* HOLD BACK on dashboard - no-op or could force exit, keep no-op */
        } else if (event->button == KEYPAD_BTN_LEFT) {
            /* fast skip left: previous */
            g_main_screen_index = (g_main_screen_index == 0) ? 1 : (g_main_screen_index - 1);
            ui_display_dashboard();
        } else if (event->button == KEYPAD_BTN_RIGHT) {
            /* fast skip right: next */
            g_main_screen_index = (g_main_screen_index + 1) % 2;
            ui_display_dashboard();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Menu root (paged) - displays menu page and accepts numeric selection
 * --------------------------------------------------------------------------- */

static void ui_display_menu_root(void)
{
    /* two pages: page 0 and page 1 */
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};

    if (g_menu_page == 0) {
        /* Page 0: 1:Temp 2:Hum | 3:Fan 4:Vent */
        snprintf(l1, sizeof(l1), "1:Temp 2:Hum 3:PerS");
        snprintf(l2, sizeof(l2), "4:Fan 5:Ven 6:Pump");
    } else {
        /* Page 1: 7:Heater 8:Light (PerSens placeholder at 3 shown earlier) */
        snprintf(l1, sizeof(l1), "7:Heater 8:Light");
        snprintf(l2, sizeof(l2), "Press #:Select");
    }
    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_menu_root_event(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS && event->type != KEYPAD_EVT_HOLD) return;

    if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9 && event->type == KEYPAD_EVT_PRESS) {
        /* numeric selection - map digit to option */
        uint8_t digit = event->button; /* 0..9 */
        switch (digit) {
            case 1:
                /* Temp per sensor selection */
                g_ui_state = UI_STATE_MENU_TEMP_SELECT;
                ui_display_temp_select();
                break;
            case 2:
                g_ui_state = UI_STATE_MENU_HUM_SELECT;
                ui_display_hum_select();
                break;
            case 3:
                g_ui_state = UI_STATE_MENU_PERSENS;
                ui_display_message_lines("Per-Sensor", "Not Implemented");
                break;
            case 4:
                /* Fan */
                g_ui_state = UI_STATE_MENU_ACT_SELECT_TYPE;
                g_act_edit_ctx.type = SYS_MGR_ACTUATOR_FANS;
                ui_display_act_select_type();
                break;
            case 5:
                g_ui_state = UI_STATE_MENU_ACT_SELECT_TYPE;
                g_act_edit_ctx.type = SYS_MGR_ACTUATOR_VENTS;
                ui_display_act_select_type();
                break;
            case 6:
                g_ui_state = UI_STATE_MENU_ACT_SELECT_TYPE;
                g_act_edit_ctx.type = SYS_MGR_ACTUATOR_PUMPS;
                ui_display_act_select_type();
                break;
            case 7:
                g_ui_state = UI_STATE_MENU_ACT_SELECT_TYPE;
                g_act_edit_ctx.type = SYS_MGR_ACTUATOR_HEATERS;
                ui_display_act_select_type();
                break;
            case 8:
                g_ui_state = UI_STATE_MENU_LIGHT_EDIT;
                /* preload light ctx from current config for edit */
                g_light_ctx.on_hour = g_ui_working_config.light_schedule.on_hour;
                g_light_ctx.on_min  = g_ui_working_config.light_schedule.on_min;
                g_light_ctx.off_hour= g_ui_working_config.light_schedule.off_hour;
                g_light_ctx.off_min = g_ui_working_config.light_schedule.off_min;
                ui_display_light_edit();
                break;
            default:
                /* ignore other digits */
                break;
        }
    } else if (event->type == KEYPAD_EVT_PRESS) {
        /* page navigation using LEFT/RIGHT/UP/DOWN */
        switch (event->button) {
            case KEYPAD_BTN_LEFT:
            case KEYPAD_BTN_UP:
                g_menu_page = (g_menu_page == 0) ? 1 : (g_menu_page - 1);
                ui_display_menu_root();
                break;
            case KEYPAD_BTN_RIGHT:
            case KEYPAD_BTN_DOWN:
                g_menu_page = (g_menu_page + 1) % 2;
                ui_display_menu_root();
                break;
            case KEYPAD_BTN_BACK:
                /* short press = erase? At root menu erase is no-op */
                break;
            case KEYPAD_BTN_ENTER:
                /* short press - no-op (we require numeric selection) */
                break;
            default:
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        /* special: HOLD BACK to exit menu fully (discard) */
        if (event->button == KEYPAD_BTN_BACK) {
            /* discard working config and return to dashboard */
            SYS_MGR_GetConfig(&g_ui_working_config);
            g_ui_state = UI_STATE_MAIN_SCREEN;
            g_main_screen_index = 0;
            ui_display_dashboard();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Temp configuration (select sensor -> edit both Tmin & Tmax on same screen)
 * --------------------------------------------------------------------------- */

static void ui_display_temp_select(void)
{
    /* Show selection prompt with available sensors */
    uint8_t sensor_count = 0;
    TempHumCtrl_GetConfiguredSensorCount(&sensor_count);

    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "Select Sensor:");
    /* show first two sensors or a compact list hint */
    if (sensor_count == 0) {
        snprintf(l2, sizeof(l2), "No sensors");
    } else {
        /* show indices in compact form */
        int displayed = 0;
        char tmp[UI_LCD_COLS + 1] = {0};
        for (uint8_t i = 0; i < sensor_count && displayed < 6; ++i) {
            char tok[6];
            snprintf(tok, sizeof(tok), "%u:S%u ", i+1, i+1);
            strncat(tmp, tok, sizeof(tmp)-strlen(tmp)-1);
            displayed++;
        }
        strncpy(l2, tmp, sizeof(l2)-1);
    }
    ui_refresh_if_changed(l1, l2);
    ui_clear_input_buffer();
}

static void ui_handle_temp_select_event(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS && event->type != KEYPAD_EVT_HOLD) return;

    /* numeric key selects sensor (1-based) */
    if (event->type == KEYPAD_EVT_PRESS && event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
        uint8_t digit = event->button;
        if (digit == 0) {
            /* treat 0 as 10 if >9 sensors? for now ignore */
            return;
        }
        uint8_t sel = digit - KEYPAD_BTN_0; /* 0..9 mapped */
        /* Our mapping used button IDs equal to digit; if your Keypad button enums differ,
           adapt the mapping here. Simpler approach: use digit directly if KeypadMgr returns 0..9. */
        uint8_t sensor_count = 0;
        TempHumCtrl_GetConfiguredSensorCount(&sensor_count);
        if (sel >= 1 && sel <= sensor_count) {
            g_selected_sensor_index = sel - 1;
            g_ui_state = UI_STATE_MENU_TEMP_EDIT;

            /* Prepopulate editing values from TempHumCtrl thresholds if available */
            float above, below;
            if (TempHumCtrl_GetTemperatureThreshold((TempHum_Sensor_ID_t)g_selected_sensor_index, &above, &below) == E_OK) {
                /* Note: API earlier specified GetTemperatureThreshold(above_out, below_out) - adjust if reversed */
                /* for safety, we will try to use g_ui_working_config per-sensor if available */
            }

            /* For simplicity, prefill buffer with current working config for this sensor if available */
            if (g_ui_working_config.per_sensor_control_enabled) {
                Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
                if (ps->temp_configured) {
                    /* format as "Tmin Tmax" on same screen; we will render fields separately */
                }
            }
            ui_display_temp_edit();
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            /* cancel -> root menu */
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

/* Display temp edit screen - single screen shows Tmin and Tmax */
static void ui_display_temp_edit(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};

    /* fetch current working values (if per_sensor_control_enabled, else use global) */
    float tmin = 20.0f, tmax = 60.0f;
    if (g_ui_working_config.per_sensor_control_enabled) {
        Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
        if (ps->temp_configured) {
            tmin = ps->temp_min_C;
            tmax = ps->temp_max_C;
        } else {
            tmin = g_ui_working_config.global_temp_min;
            tmax = g_ui_working_config.global_temp_max;
        }
    } else {
        tmin = g_ui_working_config.global_temp_min;
        tmax = g_ui_working_config.global_temp_max;
    }

    /* If there is a user edit in progress (g_input_buffer), show the buffer in the active field */
    if (g_current_field == FIELD_FIRST) {
        /* editing Tmin */
        if (g_input_index > 0) {
            snprintf(l1, sizeof(l1), "S%u Temp", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Tmin:%s Tmax:%.0f", g_input_buffer, tmax);
        } else {
            snprintf(l1, sizeof(l1), "S%u Temp", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Tmin:%.0f Tmax:%.0f", tmin, tmax);
        }
    } else {
        /* editing Tmax */
        if (g_input_index > 0) {
            snprintf(l1, sizeof(l1), "S%u Temp", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Tmin:%.0f Tmax:%s", tmin, g_input_buffer);
        } else {
            snprintf(l1, sizeof(l1), "S%u Temp", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Tmin:%.0f Tmax:%.0f", tmin, tmax);
        }
    }

    ui_refresh_if_changed(l1, l2);
}

/* Temp edit input handling */
static void ui_handle_temp_edit_event(Keypad_Event_t *event)
{
    if (event->type == KEYPAD_EVT_PRESS) {
        if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
            /* append digit */
            if (g_input_index < UI_MAX_INPUT_LEN) {
                /* convert button id to ascii digit - assumes KEYPAD_BTN_0..9 contiguous */
                char d = '0' + (char)(event->button - KEYPAD_BTN_0);
                g_input_buffer[g_input_index++] = d;
                g_input_buffer[g_input_index] = '\0';
                ui_display_temp_edit();
            }
            return;
        }
        switch (event->button) {
            case KEYPAD_BTN_LEFT:
            case KEYPAD_BTN_RIGHT:
            case KEYPAD_BTN_UP:
            case KEYPAD_BTN_DOWN:
                /* switch active field */
                g_current_field = (g_current_field == FIELD_FIRST) ? FIELD_SECOND : FIELD_FIRST;
                /* when switching, preserve or clear input buffer? keep as-is for user convenience */
                ui_display_temp_edit();
                break;
            case KEYPAD_BTN_BACK:
                /* erase last digit if exists, else do nothing */
                if (g_input_index > 0) {
                    g_input_buffer[--g_input_index] = '\0';
                    ui_display_temp_edit();
                }
                break;
            case KEYPAD_BTN_ENTER:
                /* confirm current field and either move to next or save both */
                if (g_current_field == FIELD_FIRST) {
                    /* store Tmin from buffer if present */
                    if (g_input_index > 0) {
                        float new_tmin = strtof(g_input_buffer, NULL);
                        /* simple clamp per user rule 20..60 */
                        if (new_tmin < 20.0f || new_tmin > 60.0f) {
                            ui_display_message_lines("Invalid Tmin", "Range 20-60");
                            ui_clear_input_buffer();
                            return;
                        } else {
                            /* apply to working config */
                            if (g_ui_working_config.per_sensor_control_enabled) {
                                Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
                                ps->temp_min_C = new_tmin;
                                ps->temp_configured = true;
                            } else {
                                g_ui_working_config.global_temp_min = new_tmin;
                            }
                            /* move to Tmax editing */
                            g_current_field = FIELD_SECOND;
                            ui_clear_input_buffer();
                            ui_display_temp_edit();
                        }
                    } else {
                        /* nothing entered - treat as no-op */
                        g_current_field = FIELD_SECOND;
                        ui_display_temp_edit();
                    }
                } else {
                    /* FIELD_SECOND - Tmax - finalize and save */
                    if (g_input_index > 0) {
                        float new_tmax = strtof(g_input_buffer, NULL);
                        if (new_tmax < 20.0f || new_tmax > 60.0f) {
                            ui_display_message_lines("Invalid Tmax", "Range 20-60");
                            ui_clear_input_buffer();
                            return;
                        } else {
                            /* Check min < max constraint */
                            float current_min = g_ui_working_config.per_sensor_control_enabled
                                ? g_ui_working_config.per_sensor[g_selected_sensor_index].temp_min_C
                                : g_ui_working_config.global_temp_min;
                            if (new_tmax <= current_min) {
                                ui_display_message_lines("Invalid Range", "Tmax must > Tmin");
                                ui_clear_input_buffer();
                                return;
                            }
                            /* save */
                            if (g_ui_working_config.per_sensor_control_enabled) {
                                Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
                                ps->temp_max_C = new_tmax;
                                ps->temp_configured = true;
                            } else {
                                g_ui_working_config.global_temp_max = new_tmax;
                            }
                            /* commit to TempHumCtrl if desired - better to call at Save & Exit; but we can optionally set per-sensor thresholds */
                            /* Return to menu root after saving local change */
                            ui_display_message_lines("Temp Saved", "Returning...");
                            /* small delay could be added if desired; here we immediately go back */
                            g_ui_state = UI_STATE_MENU_ROOT;
                            ui_display_menu_root();
                            ui_clear_input_buffer();
                        }
                    } else {
                        /* no input - simply return */
                        g_ui_state = UI_STATE_MENU_ROOT;
                        ui_display_menu_root();
                    }
                }
                break;
            default:
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            /* cancel */
            ui_clear_input_buffer();
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Hum configuration (select sensor -> edit both Hmin & Hmax on same screen)
 * --------------------------------------------------------------------------- */

static void ui_display_hum_select(void)
{
    uint8_t sensor_count = 0;
    TempHumCtrl_GetConfiguredSensorCount(&sensor_count);

    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "Select Sensor:");
    if (sensor_count == 0) {
        snprintf(l2, sizeof(l2), "No sensors");
    } else {
        char tmp[UI_LCD_COLS + 1] = {0};
        int displayed = 0;
        for (uint8_t i = 0; i < sensor_count && displayed < 6; ++i) {
            char tok[6];
            snprintf(tok, sizeof(tok), "%u:S%u ", i+1, i+1);
            strncat(tmp, tok, sizeof(tmp)-strlen(tmp)-1);
            displayed++;
        }
        strncpy(l2, tmp, sizeof(l2)-1);
    }
    ui_refresh_if_changed(l1, l2);
    ui_clear_input_buffer();
}

static void ui_handle_hum_select_event(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS && event->type != KEYPAD_EVT_HOLD) return;

    if (event->type == KEYPAD_EVT_PRESS && event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
        uint8_t digit = event->button;
        uint8_t sel = digit - KEYPAD_BTN_0;
        uint8_t sensor_count = 0;
        TempHumCtrl_GetConfiguredSensorCount(&sensor_count);
        if (sel >= 1 && sel <= sensor_count) {
            g_selected_sensor_index = sel - 1;
            /* If sensor does not support humidity, inform user */
            float hum;
            Status_t st = TempHumCtrl_GetHumidity((TempHum_Sensor_ID_t)g_selected_sensor_index, &hum);
            if (st == E_NOT_SUPPORTED) {
                ui_display_message_lines("Sensor has no", "humidity");
                /* return to menu after a short message */
                g_ui_state = UI_STATE_MENU_ROOT;
                ui_display_menu_root();
            } else {
                g_ui_state = UI_STATE_MENU_HUM_EDIT;
                ui_display_hum_edit();
            }
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

static void ui_display_hum_edit(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};

    /* fetch existing working values */
    float hmin = 20.0f, hmax = 60.0f;
    if (g_ui_working_config.per_sensor_control_enabled) {
        Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
        if (ps->hum_configured) {
            hmin = ps->hum_min_P;
            hmax = ps->hum_max_P;
        } else {
            hmin = g_ui_working_config.global_hum_min;
            hmax = g_ui_working_config.global_hum_max;
        }
    } else {
        hmin = g_ui_working_config.global_hum_min;
        hmax = g_ui_working_config.global_hum_max;
    }

    if (g_current_field == FIELD_FIRST) {
        if (g_input_index > 0) {
            snprintf(l1, sizeof(l1), "S%u Hum", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Hmin:%s Hmax:%.0f", g_input_buffer, hmax);
        } else {
            snprintf(l1, sizeof(l1), "S%u Hum", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Hmin:%.0f Hmax:%.0f", hmin, hmax);
        }
    } else {
        if (g_input_index > 0) {
            snprintf(l1, sizeof(l1), "S%u Hum", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Hmin:%.0f Hmax:%s", hmin, g_input_buffer);
        } else {
            snprintf(l1, sizeof(l1), "S%u Hum", g_selected_sensor_index + 1);
            snprintf(l2, sizeof(l2), "Hmin:%.0f Hmax:%.0f", hmin, hmax);
        }
    }

    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_hum_edit_event(Keypad_Event_t *event)
{
    if (event->type == KEYPAD_EVT_PRESS) {
        if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
            if (g_input_index < UI_MAX_INPUT_LEN) {
                char d = '0' + (char)(event->button - KEYPAD_BTN_0);
                g_input_buffer[g_input_index++] = d;
                g_input_buffer[g_input_index] = '\0';
                ui_display_hum_edit();
            }
            return;
        }
        switch (event->button) {
            case KEYPAD_BTN_LEFT:
            case KEYPAD_BTN_RIGHT:
            case KEYPAD_BTN_UP:
            case KEYPAD_BTN_DOWN:
                g_current_field = (g_current_field == FIELD_FIRST) ? FIELD_SECOND : FIELD_FIRST;
                ui_display_hum_edit();
                break;
            case KEYPAD_BTN_BACK:
                if (g_input_index > 0) {
                    g_input_buffer[--g_input_index] = '\0';
                    ui_display_hum_edit();
                }
                break;
            case KEYPAD_BTN_ENTER:
                if (g_current_field == FIELD_FIRST) {
                    if (g_input_index > 0) {
                        float new_hmin = strtof(g_input_buffer, NULL);
                        if (new_hmin < 20.0f || new_hmin > 60.0f) {
                            ui_display_message_lines("Invalid Hmin", "Range 20-60");
                            ui_clear_input_buffer();
                            return;
                        } else {
                            if (g_ui_working_config.per_sensor_control_enabled) {
                                Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
                                ps->hum_min_P = new_hmin;
                                ps->hum_configured = true;
                            } else {
                                g_ui_working_config.global_hum_min = new_hmin;
                            }
                            g_current_field = FIELD_SECOND;
                            ui_clear_input_buffer();
                            ui_display_hum_edit();
                        }
                    } else {
                        g_current_field = FIELD_SECOND;
                        ui_display_hum_edit();
                    }
                } else {
                    if (g_input_index > 0) {
                        float new_hmax = strtof(g_input_buffer, NULL);
                        if (new_hmax < 20.0f || new_hmax > 60.0f) {
                            ui_display_message_lines("Invalid Hmax", "Range 20-60");
                            ui_clear_input_buffer();
                            return;
                        } else {
                            float current_min = g_ui_working_config.per_sensor_control_enabled
                                ? g_ui_working_config.per_sensor[g_selected_sensor_index].hum_min_P
                                : g_ui_working_config.global_hum_min;
                            if (new_hmax <= current_min) {
                                ui_display_message_lines("Invalid Range", "Hmax must > Hmin");
                                ui_clear_input_buffer();
                                return;
                            }
                            if (g_ui_working_config.per_sensor_control_enabled) {
                                Per_Sensor_Config_t *ps = &g_ui_working_config.per_sensor[g_selected_sensor_index];
                                ps->hum_max_P = new_hmax;
                                ps->hum_configured = true;
                            } else {
                                g_ui_working_config.global_hum_max = new_hmax;
                            }
                            ui_display_message_lines("Hum Saved", "Returning...");
                            g_ui_state = UI_STATE_MENU_ROOT;
                            ui_display_menu_root();
                            ui_clear_input_buffer();
                        }
                    } else {
                        g_ui_state = UI_STATE_MENU_ROOT;
                        ui_display_menu_root();
                    }
                }
                break;
            default:
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            ui_clear_input_buffer();
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Actuator selection and editing
 * --------------------------------------------------------------------------- */

/* helper: build readable actuator type name */
static const char* actuator_type_to_str(SYS_MGR_Actuator_t t)
{
    switch (t) {
        case SYS_MGR_ACTUATOR_FANS: return "Fan";
        case SYS_MGR_ACTUATOR_HEATERS: return "Heat";
        case SYS_MGR_ACTUATOR_PUMPS: return "Pump";
        case SYS_MGR_ACTUATOR_VENTS: return "Vent";
        case SYS_MGR_ACTUATOR_LIGHTS: return "Light";
        default: return "Act";
    }
}

static void ui_display_act_select_type(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "%s Config", actuator_type_to_str(g_act_edit_ctx.type));
    snprintf(l2, sizeof(l2), "Select ID #: 1..n");
    ui_refresh_if_changed(l1, l2);
}

/* For selecting actuator ID, we will query *_ID_COUNT macros via SysMgr config or controller APIs.
   For simplicity, we'll assume SysMgr config has knowledge of counts (not present in sys_mgr.h),
   so we will attempt to iterate until a controller GetState returns E_INVALID_PARAM or similar.
   Here, to keep it simple, we'll allow selecting IDs 1..8 (most systems have fewer). */
static void ui_display_act_select_id(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "Select %s ID", actuator_type_to_str(g_act_edit_ctx.type));
    snprintf(l2, sizeof(l2), "1:ID1 2:ID2 3:ALL");
    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_act_select_type_event(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS && event->type != KEYPAD_EVT_HOLD) return;

    if (event->type == KEYPAD_EVT_PRESS) {
        if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
            uint8_t digit = event->button - KEYPAD_BTN_0;
            if (digit == 0) return;
            /* choose first ID = digit mapping */
            g_act_edit_ctx.actuator_id = digit - 1; /* zero based */
            g_ui_state = UI_STATE_MENU_ACT_MODE;
            ui_display_act_mode();
        } else if (event->button == KEYPAD_BTN_BACK) {
            /* short press => go back to menu root */
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

static void ui_display_act_mode(void)
{
    char l1[UI_LCD_COLS + 1];
    char l2[UI_LCD_COLS + 1];
    snprintf(l1, sizeof(l1), "%s%u Mode?", actuator_type_to_str(g_act_edit_ctx.type), g_act_edit_ctx.actuator_id + 1);
    snprintf(l2, sizeof(l2), "1:Auto 2:Manual");
    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_act_mode_event(Keypad_Event_t *event)
{
    if (event->type != KEYPAD_EVT_PRESS) return;

    if (event->button == KEYPAD_BTN_1) {
        /* set auto mode for this actuator in working config */
        /* Implementation detail:
           map g_act_edit_ctx.type & actuator_id into g_ui_working_config actuator cycle / manual flag fields.
           For now, set cycle.enabled = false to reflect auto.
         */
        switch (g_act_edit_ctx.type) {
            case SYS_MGR_ACTUATOR_FANS:
                g_ui_working_config.fans_cycle.enabled = false;
                break;
            case SYS_MGR_ACTUATOR_HEATERS:
                g_ui_working_config.heaters_cycle.enabled = false;
                break;
            case SYS_MGR_ACTUATOR_PUMPS:
                g_ui_working_config.pumps_cycle.enabled = false;
                break;
            case SYS_MGR_ACTUATOR_VENTS:
                g_ui_working_config.vents_cycle.enabled = false;
                break;
            default:
                break;
        }
        ui_display_message_lines("Set to Auto", "Returning...");
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    } else if (event->button == KEYPAD_BTN_2) {
        /* Manual mode -> edit on/off times */
        /* initialize edit context values: read existing cycle if present */
        g_act_edit_ctx.unit_on = TIME_UNIT_SECONDS;
        g_act_edit_ctx.unit_off = TIME_UNIT_SECONDS;
        /* prefill with current cycle if available */
        Actuator_Cycle_t *cycle = NULL;
        switch (g_act_edit_ctx.type) {
            case SYS_MGR_ACTUATOR_FANS: cycle = &g_ui_working_config.fans_cycle; break;
            case SYS_MGR_ACTUATOR_HEATERS: cycle = &g_ui_working_config.heaters_cycle; break;
            case SYS_MGR_ACTUATOR_PUMPS: cycle = &g_ui_working_config.pumps_cycle; break;
            case SYS_MGR_ACTUATOR_VENTS: cycle = &g_ui_working_config.vents_cycle; break;
            default: break;
        }
        if (cycle && cycle->enabled) {
            g_act_edit_ctx.on_value = cycle->on_time_sec;
            g_act_edit_ctx.off_value = cycle->off_time_sec;
        } else {
            g_act_edit_ctx.on_value = 30;
            g_act_edit_ctx.off_value = 60;
        }
        g_ui_state = UI_STATE_MENU_ACT_MANUAL_EDIT;
        g_current_field = FIELD_FIRST;
        ui_clear_input_buffer();
        ui_display_act_manual_edit();
    } else if (event->button == KEYPAD_BTN_BACK) {
        g_ui_state = UI_STATE_MENU_ROOT;
        ui_display_menu_root();
    }
}

/* Actuator manual edit display: show ON on line1, OFF on line2.
   If g_input_buffer has data, show it in the active field (first or second).
   Use ui_format_seconds_by_unit for unit-aware display. */
static void ui_display_act_manual_edit(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    char v1[16] = {0};
    char v2[16] = {0};

    /* If user is editing active field, show buffer; else show formatted values */
    if (g_current_field == FIELD_FIRST && g_input_index > 0) {
        snprintf(v1, sizeof(v1), "%s%s", g_input_buffer, TimeUnitStr[g_act_edit_ctx.unit_on]);
    } else {
        ui_format_seconds_by_unit(v1, sizeof(v1), g_act_edit_ctx.on_value, g_act_edit_ctx.unit_on);
    }
    if (g_current_field == FIELD_SECOND && g_input_index > 0) {
        snprintf(v2, sizeof(v2), "%s%s", g_input_buffer, TimeUnitStr[g_act_edit_ctx.unit_off]);
    } else {
        ui_format_seconds_by_unit(v2, sizeof(v2), g_act_edit_ctx.off_value, g_act_edit_ctx.unit_off);
    }

    snprintf(l1, sizeof(l1), "%s%u ON:%s", actuator_type_to_str(g_act_edit_ctx.type), g_act_edit_ctx.actuator_id + 1, v1);
    snprintf(l2, sizeof(l2), "OFF:%s", v2);
    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_act_manual_edit_event(Keypad_Event_t *event)
{
    if (event->type == KEYPAD_EVT_PRESS) {
        /* numeric input */
        if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
            if (g_input_index < UI_MAX_INPUT_LEN) {
                char d = '0' + (char)(event->button - KEYPAD_BTN_0);
                g_input_buffer[g_input_index++] = d;
                g_input_buffer[g_input_index] = '\0';
                ui_display_act_manual_edit();
            }
            return;
        }

        switch (event->button) {
            case KEYPAD_BTN_LEFT:
            case KEYPAD_BTN_RIGHT:
            case KEYPAD_BTN_UP:
            case KEYPAD_BTN_DOWN:
                /* toggle active field */
                g_current_field = (g_current_field == FIELD_FIRST) ? FIELD_SECOND : FIELD_FIRST;
                ui_clear_input_buffer(); /* optional: clear buffer when switching fields */
                ui_display_act_manual_edit();
                break;
            case KEYPAD_BTN_BACK:
                if (g_input_index > 0) {
                    g_input_buffer[--g_input_index] = '\0';
                    ui_display_act_manual_edit();
                } else {
                    /* short back with empty buffer -> no-op */
                }
                break;
            case KEYPAD_BTN_ENTER:
                /* short press confirm: save values */
                if (g_current_field == FIELD_FIRST) {
                    /* if editing first field but no input, accept existing on_value */
                    if (g_input_index > 0) {
                        /* parse input buffer into seconds according to unit */
                        uint32_t val_seconds = 0;
                        if (!ui_parse_input_to_uint32(&val_seconds, g_act_edit_ctx.unit_on,
                                                      (g_act_edit_ctx.unit_on == TIME_UNIT_SECONDS) ? 999 :
                                                      (g_act_edit_ctx.unit_on == TIME_UNIT_MINUTES) ? 720 : 12)) {
                            ui_display_message_lines("Invalid ON val", "Try again");
                            ui_clear_input_buffer();
                            return;
                        }
                        /* convert to seconds and assign */
                        g_act_edit_ctx.on_value = val_seconds;
                    }
                    /* move to OFF for confirmation or allow saving directly */
                    g_current_field = FIELD_SECOND;
                    ui_clear_input_buffer();
                    ui_display_act_manual_edit();
                } else {
                    /* FIELD_SECOND: commit OFF value and save both */
                    if (g_input_index > 0) {
                        uint32_t val_seconds = 0;
                        if (!ui_parse_input_to_uint32(&val_seconds, g_act_edit_ctx.unit_off,
                                                      (g_act_edit_ctx.unit_off == TIME_UNIT_SECONDS) ? 999 :
                                                      (g_act_edit_ctx.unit_off == TIME_UNIT_MINUTES) ? 720 : 12)) {
                            ui_display_message_lines("Invalid OFF val", "Try again");
                            ui_clear_input_buffer();
                            return;
                        }
                        g_act_edit_ctx.off_value = val_seconds;
                    }
                    /* Validate >0 */
                    if (g_act_edit_ctx.on_value == 0 || g_act_edit_ctx.off_value == 0) {
                        ui_display_message_lines("Values must be", ">= 1 unit");
                        return;
                    }
                    /* Save into working config for the actuator type */
                    Actuator_Cycle_t *cycle = NULL;
                    switch (g_act_edit_ctx.type) {
                        case SYS_MGR_ACTUATOR_FANS: cycle = &g_ui_working_config.fans_cycle; break;
                        case SYS_MGR_ACTUATOR_HEATERS: cycle = &g_ui_working_config.heaters_cycle; break;
                        case SYS_MGR_ACTUATOR_PUMPS: cycle = &g_ui_working_config.pumps_cycle; break;
                        case SYS_MGR_ACTUATOR_VENTS: cycle = &g_ui_working_config.vents_cycle; break;
                        default: break;
                    }
                    if (cycle) {
                        cycle->enabled = true;
                        cycle->on_time_sec = g_act_edit_ctx.on_value;
                        cycle->off_time_sec = g_act_edit_ctx.off_value;
                    }
                    ui_display_message_lines("Actuator Saved", "Returning...");
                    g_ui_state = UI_STATE_MENU_ROOT;
                    ui_display_menu_root();
                    ui_clear_input_buffer();
                }
                break;
            default:
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_ENTER) {
            /* Toggle unit for currently active field */
            if (g_current_field == FIELD_FIRST) {
                g_act_edit_ctx.unit_on = (TimeUnit_t)((g_act_edit_ctx.unit_on + 1) % TIME_UNIT_COUNT);
            } else {
                g_act_edit_ctx.unit_off = (TimeUnit_t)((g_act_edit_ctx.unit_off + 1) % TIME_UNIT_COUNT);
            }
            ui_display_act_manual_edit();
        } else if (event->button == KEYPAD_BTN_RIGHT) {
            /* switch to next actuator */
            /* For simplicity increment id and wrap at some max (e.g., 8). In a real implementation,
               query the controller for actual ID_COUNT and wrap appropriately. */
            g_act_edit_ctx.actuator_id++;
            if (g_act_edit_ctx.actuator_id >= 8) g_act_edit_ctx.actuator_id = 0;
            ui_display_act_manual_edit();
        } else if (event->button == KEYPAD_BTN_LEFT) {
            /* previous actuator */
            if (g_act_edit_ctx.actuator_id == 0) g_act_edit_ctx.actuator_id = 7;
            else g_act_edit_ctx.actuator_id--;
            ui_display_act_manual_edit();
        } else if (event->button == KEYPAD_BTN_BACK) {
            /* hold back cancels */
            ui_clear_input_buffer();
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Light schedule editing
 * --------------------------------------------------------------------------- */

static void ui_display_light_edit(void)
{
    char l1[UI_LCD_COLS + 1] = {0};
    char l2[UI_LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "Light ON:%02u:%02u", g_light_ctx.on_hour, g_light_ctx.on_min);
    snprintf(l2, sizeof(l2), "OFF:%02u:%02u", g_light_ctx.off_hour, g_light_ctx.off_min);
    ui_refresh_if_changed(l1, l2);
}

static void ui_handle_light_edit_event(Keypad_Event_t *event)
{
    static uint8_t edit_pos = 0; /* 0=ON_HH,1=ON_MM,2=OFF_HH,3=OFF_MM */
    if (event->type == KEYPAD_EVT_PRESS) {
        if (event->button >= KEYPAD_BTN_0 && event->button <= KEYPAD_BTN_9) {
            /* append digit to current field */
            char d = '0' + (char)(event->button - KEYPAD_BTN_0);
            /* implement simple stateful edit with 2-digit fields */
            /* Build temp string per field */
            char tmp[3] = {0};
            uint8_t val = 0;
            switch (edit_pos) {
                case 0: /* ON_HH */
                    tmp[0] = (g_light_ctx.on_hour / 10) + '0';
                    tmp[1] = (g_light_ctx.on_hour % 10) + '0';
                    tmp[2] = '\0';
                    /* shift digits left and append */
                    tmp[0] = tmp[1];
                    tmp[1] = d;
                    val = (uint8_t)atoi(tmp);
                    if (val <= 23) g_light_ctx.on_hour = val;
                    ui_display_light_edit();
                    break;
                case 1: /* ON_MM */
                    tmp[0] = (g_light_ctx.on_min / 10) + '0';
                    tmp[1] = (g_light_ctx.on_min % 10) + '0';
                    tmp[0] = tmp[1];
                    tmp[1] = d;
                    val = (uint8_t)atoi(tmp);
                    if (val <= 59) g_light_ctx.on_min = val;
                    ui_display_light_edit();
                    break;
                case 2: /* OFF_HH */
                    tmp[0] = (g_light_ctx.off_hour / 10) + '0';
                    tmp[1] = (g_light_ctx.off_hour % 10) + '0';
                    tmp[0] = tmp[1];
                    tmp[1] = d;
                    val = (uint8_t)atoi(tmp);
                    if (val <= 23) g_light_ctx.off_hour = val;
                    ui_display_light_edit();
                    break;
                case 3: /* OFF_MM */
                    tmp[0] = (g_light_ctx.off_min / 10) + '0';
                    tmp[1] = (g_light_ctx.off_min % 10) + '0';
                    tmp[0] = tmp[1];
                    tmp[1] = d;
                    val = (uint8_t)atoi(tmp);
                    if (val <= 59) g_light_ctx.off_min = val;
                    ui_display_light_edit();
                    break;
            }
            return;
        }

        switch (event->button) {
            case KEYPAD_BTN_LEFT:
                edit_pos = (edit_pos == 0) ? 3 : edit_pos - 1;
                break;
            case KEYPAD_BTN_RIGHT:
                edit_pos = (edit_pos + 1) % 4;
                break;
            case KEYPAD_BTN_BACK:
                /* erase last digit: rudimentary - set field to zero */
                switch (edit_pos) {
                    case 0: g_light_ctx.on_hour = 0; break;
                    case 1: g_light_ctx.on_min = 0; break;
                    case 2: g_light_ctx.off_hour = 0; break;
                    case 3: g_light_ctx.off_min = 0; break;
                }
                ui_display_light_edit();
                break;
            case KEYPAD_BTN_ENTER:
                /* confirm: validate then save */
                if (!validate_light_times(g_light_ctx.on_hour, g_light_ctx.on_min, g_light_ctx.off_hour, g_light_ctx.off_min)) {
                    ui_display_message_lines("Invalid times", "ON != OFF");
                    return;
                }
                /* save into working config */
                g_ui_working_config.light_schedule.on_hour = g_light_ctx.on_hour;
                g_ui_working_config.light_schedule.on_min = g_light_ctx.on_min;
                g_ui_working_config.light_schedule.off_hour = g_light_ctx.off_hour;
                g_ui_working_config.light_schedule.off_min = g_light_ctx.off_min;
                g_ui_working_config.light_schedule.enabled = true;
                ui_display_message_lines("Light Saved", "Returning...");
                g_ui_state = UI_STATE_MENU_ROOT;
                ui_display_menu_root();
                break;
            default:
                break;
        }
    } else if (event->type == KEYPAD_EVT_HOLD) {
        if (event->button == KEYPAD_BTN_BACK) {
            /* cancel edit */
            SYS_MGR_GetConfig(&g_ui_working_config);
            g_ui_state = UI_STATE_MENU_ROOT;
            ui_display_menu_root();
        }
    }
}

/* ---------------------------------------------------------------------------
 * Utility functions: parse and format
 * --------------------------------------------------------------------------- */

/* Parse decimal numeric input buffer (string) into seconds given a unit.
   max_allowed is max allowed value in the input unit (e.g., 999 for seconds).
   Returns true on success, out_val is seconds. */
static bool ui_parse_input_to_uint32(uint32_t *out_val, TimeUnit_t unit, uint32_t max_allowed)
{
    if (!out_val) return false;
    if (g_input_index == 0) return false; /* no input */
    char tmp[UI_MAX_INPUT_LEN + 1];
    strncpy(tmp, g_input_buffer, sizeof(tmp));
    tmp[UI_MAX_INPUT_LEN] = '\0';
    uint32_t v = (uint32_t)atoi(tmp);
    if (v == 0) return false;
    if (v > max_allowed) return false;
    uint32_t seconds = v;
    if (unit == TIME_UNIT_SECONDS) {
        seconds = v;
    } else if (unit == TIME_UNIT_MINUTES) {
        seconds = v * 60U;
    } else if (unit == TIME_UNIT_HOURS) {
        seconds = v * 3600U;
    }
    *out_val = seconds;
    return true;
}

/* Format seconds into a string according to unit (value part only) */
static void ui_format_seconds_by_unit(char *buf, size_t buf_sz, uint32_t seconds, TimeUnit_t unit)
{
    if (!buf) return;
    if (unit == TIME_UNIT_SECONDS) {
        snprintf(buf, buf_sz, "%03u%s", (unsigned int)seconds, TimeUnitStr[unit]);
    } else if (unit == TIME_UNIT_MINUTES) {
        uint32_t m = seconds / 60U;
        snprintf(buf, buf_sz, "%03u%s", (unsigned int)m, TimeUnitStr[unit]);
    } else {
        uint32_t h = seconds / 3600U;
        snprintf(buf, buf_sz, "%02u%s", (unsigned int)h, TimeUnitStr[unit]);
    }
}

/* ---------------------------------------------------------------------------
 * Validation helpers
 * --------------------------------------------------------------------------- */

static bool validate_temp_range(float min_c, float max_c)
{
    if (min_c < 20.0f || max_c > 60.0f || min_c >= max_c) return false;
    return true;
}
static bool validate_hum_range(float min_p, float max_p)
{
    if (min_p < 20.0f || max_p > 60.0f || min_p >= max_p) return false;
    return true;
}
static bool validate_light_times(uint8_t on_h, uint8_t on_m, uint8_t off_h, uint8_t off_m)
{
    if (on_h > 23 || off_h > 23 || on_m > 59 || off_m > 59) return false;
    if (on_h == off_h && on_m == off_m) return false; /* cannot be equal */
    return true;
}

/* ---------------------------------------------------------------------------
 * Save helper - writes working config back to SysMgr
 * --------------------------------------------------------------------------- */

static void ui_save_working_config(void)
{
    /* Validate the entire working configuration using SYS_MGR_ValidateConfig if needed */
    if (SYS_MGR_ValidateConfig(&g_ui_working_config) != E_OK) {
        ui_display_message_lines("Config Invalid", "Not saved");
        return;
    }
    if (SYS_MGR_UpdateConfigRuntime(&g_ui_working_config) != E_OK) {
        ui_display_message_lines("Update Failed", "Check system");
        return;
    }
    if (SYS_MGR_SaveConfigToFlash() != E_OK) {
        ui_display_message_lines("Save Failed", "Check flash");
        return;
    }
    ui_display_message_lines("Config Saved", "Returning...");
    /* reload to ensure working copy matches persisted */
    SYS_MGR_GetConfig(&g_ui_working_config);
}

/* End of file */
