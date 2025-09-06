/**
 * @file ui_manager.c
 * @brief Full UI Manager implementation (final)
 * @version 4.0
 * @date 2025
 *
 * Full implementation of UI state machine:
 *  - Dashboard screens (rotating): System Average, Per-Sensor (rotates through all),
 *    Actuator states, Mode & Time
 *  - Configuration menu (entered by HOLD ENTER)
 *    - 1: Global Temp (Tmin/Tmax) edit (min then max in same flow)
 *    - 2: Global Hum (Hmin/Hmax) edit
 *    - 3: Per-Sensor (placeholder)
 *    - 4: Fan config (per-fan auto/manual and manual ON/OFF cycle)
 *    - 5: Vent config (same)
 *    - 6: Pump config (same)
 *    - 7: Heater config (same)
 *    - 8: Light schedule (ON/ OFF time)
 *    - 9: Save & Exit (commits working configuration)
 *
 * Input rules:
 *  - Numeric keys (0..9) append digits to numeric input
 *  - BACK short = erase last digit; BACK hold = cancel & return to menu root (discard changes)
 *  - ENTER short = accept current field / move to next (e.g., Tmin -> Tmax)
 *  - ENTER hold (only in actuator manual edit) = toggle unit (s/m/h)
 *  - HOLD ENTER (on dashboard) = enter menu root
 *  - LEFT/RIGHT in dashboard = manual page navigation
 *  - LEFT/RIGHT hold in actuator manual edit = move to previous/next actuator ID
 *  - Menu timeout (UI_MENU_TIMEOUT_MS) discards working copy and returns to dashboard
 *
 * Assumptions & dependencies (must exist in project):
 *  - KeypadMgr_GetEvent(Keypad_Event_t *out)
 *  - TempHumCtrl_* APIs described earlier
 *  - SysMgr APIs: SYS_MGR_GetConfig, SYS_MGR_UpdateConfigRuntime, SYS_MGR_SaveConfigToFlash, SYS_MGR_ValidateConfig
 *  - Actuator CTRL APIs: FanCtrl_SetState/GetState, HeaterCtrl_..., PumpCtrl_..., VenCtrl_..., LightCtrl_...
 *  - ui_manager_cfg.* wrappers: ui_display_message(), ui_display_numeric_input(), ui_display_clear(), UI_MGR_GetTick()
 *  - actuator count macros: FAN_ID_COUNT, VEN_ID_COUNT, PUMP_ID_COUNT, HEATER_ID_COUNT, LIGHT_ID_COUNT
 *  - TEMPHUM_SENSOR_ID_COUNT
 *
 * Implementation notes:
 *  - This file uses the 'working configuration' pattern: UI modifies a copy of the current config,
 *    user saves with Save & Exit (or individual screens call commit if desired).
 *  - All numeric validation is performed before committing.
 *  - Code is split into small handler functions for readability and easy extension.
 */

#include "ui_manager.h"
#include "ui_manager_cfg.h"

#include "keypad_mgr.h"
#include "temphumctrl.h"
#include "sys_mgr.h"
#include "fanctrl.h"
#include "venctrl.h"
#include "pumpctrl.h"
#include "heaterctrl.h"
#include "lightctrl.h"
#include "logger.h"
#include "sys_mgr_core.h"
#include "sys_mgr_cfg.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "UI_MGR_FINAL";

/* ------------------------------------------------------------------------- */
/* Internal constants & types                                                */
/* ------------------------------------------------------------------------- */

#define LCD_COLS (UI_LCD_COLS)
#define LCD_ROWS (UI_LCD_ROWS)

/* Max input lengths & limits (kept in config header as well) */
#define INPUT_MAX_LEN UI_MAX_INPUT_LEN
#define ACT_MAX_SEC UI_ACTUATOR_MAX_SEC
#define ACT_MAX_MIN UI_ACTUATOR_MAX_MIN
#define ACT_MAX_HOUR UI_ACTUATOR_MAX_HOUR

static const char *TIME_UNIT_LABELS[UNIT_COUNT] = {"s", "m", "h"};
/* ------------------------------------------------------------------------- */
/* Internal UI state                                                         */
/* ------------------------------------------------------------------------- */

/* Working copy of configuration that UI edits; commit via Save & Exit */
static SysMgr_Config_t g_working_cfg;

/* Input buffer for numeric entry (digits only) */
static char g_input_buf[INPUT_MAX_LEN + 1];
static int g_input_len = 0;

/* UI state */
static UI_State_t g_state = UI_STATE_MAIN_SCREEN;
static DashPage_t g_dash_page = DASH_PAGE_AVG;
static uint32_t g_last_display_ms = 0;
static uint32_t g_last_key_ms = 0;

/* Dashboard state */
static uint16_t g_sensor_index = 0; /* index of sensor shown on sensor page */

/* Editing flags */
static bool g_editing_min = true; /* for min/max pairs (start editing min) */
static uint8_t g_field_pos = 0;   /* 0 = first field (min/on), 1 = second field (max/off) */

static ActuatorEditCtx_t g_act_ctx;

static LightEditCtx_t g_light_ctx;

/* Cached last drawn screen lines to avoid redundant writes */
static char s_last_l1[LCD_COLS + 1];
static char s_last_l2[LCD_COLS + 1];

/* ------------------------------------------------------------------------- */
/* Forward declarations                                                      */
/* ------------------------------------------------------------------------- */

/* Rendering */
static void render_dashboard(void);
static void render_dash_avg(void);
static void render_dash_sensor(void);
static void render_dash_actuators(void);
static void render_dash_mode_time(void);

static void render_menu_root(void);
static void render_edit_temp_global(void);
static void render_edit_hum_global(void);
static void render_actuator_select_id(void);
static void render_actuator_mode(void);
static void render_actuator_manual_edit(void);
static void render_light_edit(void);

/* Event handlers */
static void handle_event_main(const Keypad_Event_t *ev);
static void handle_event_menu_root(const Keypad_Event_t *ev);
static void handle_event_edit_temp(const Keypad_Event_t *ev);
static void handle_event_edit_hum(const Keypad_Event_t *ev);
static void handle_event_actuator(const Keypad_Event_t *ev);
static void handle_event_light(const Keypad_Event_t *ev);

/* Input helpers */
static void input_clear(void);
static void input_append_digit(char d);
static void input_erase_last(void);
static bool input_is_empty(void);

/* Utility */
static void lcd_write_if_changed(const char *l1, const char *l2);
static uint32_t to_seconds_from_unit(uint32_t value, TimeUnit_t u);
static uint32_t from_seconds_to_unit_value(uint32_t seconds, TimeUnit_t u);
static void format_timeunit_value(uint32_t seconds, TimeUnit_t u, char *out, size_t out_sz);

/* Config helpers */
static void load_working_cfg(void);
static void discard_working_cfg(void);
static Status_t commit_working_cfg(void);

/* ------------------------------------------------------------------------- */
/* Implementations                                                           */
/* ------------------------------------------------------------------------- */

/* --- Input helpers --- */

static void input_clear(void)
{
    g_input_buf[0] = '\0';
    g_input_len = 0;
}

static void input_append_digit(char d)
{
    if (g_input_len < INPUT_MAX_LEN)
    {
        g_input_buf[g_input_len++] = d;
        g_input_buf[g_input_len] = '\0';
    }
}
static void input_erase_last(void)
{
    if (g_input_len > 0)
    {
        g_input_buf[--g_input_len] = '\0';
    }
}
static bool input_is_empty(void)
{
    return (g_input_len == 0);
}

/* --- Utility helpers --- */

/* Only write LCD when content changed (reduces flicker) */
static void lcd_write_if_changed(const char *l1, const char *l2)
{
    char tmp1[LCD_COLS + 1] = {0};
    char tmp2[LCD_COLS + 1] = {0};

    if (l1)
        strncpy(tmp1, l1, LCD_COLS);
    if (l2)
        strncpy(tmp2, l2, LCD_COLS);

    if (strncmp(tmp1, s_last_l1, LCD_COLS) == 0 && strncmp(tmp2, s_last_l2, LCD_COLS) == 0)
    {
        return; /* unchanged */
    }

    /* write */
    ui_display_message(tmp1, tmp2);

    /* cache */
    memset(s_last_l1, 0, sizeof(s_last_l1));
    memset(s_last_l2, 0, sizeof(s_last_l2));
    strncpy(s_last_l1, tmp1, LCD_COLS);
    strncpy(s_last_l2, tmp2, LCD_COLS);
}

/* Convert value in given unit to seconds (value must be integer) */
static uint32_t to_seconds_from_unit(uint32_t value, TimeUnit_t u)
{
    switch (u)
    {
    case UNIT_SEC:
        return value;
    case UNIT_MIN:
        return value * 60U;
    case UNIT_HOUR:
        return value * 3600U;
    default:
        return value;
    }
}

/* Convert seconds to value in chosen unit (integer truncation) */
static uint32_t from_seconds_to_unit_value(uint32_t seconds, TimeUnit_t u)
{
    switch (u)
    {
    case UNIT_SEC:
        return seconds;
    case UNIT_MIN:
        return seconds / 60U;
    case UNIT_HOUR:
        return seconds / 3600U;
    default:
        return seconds;
    }
}

/* Format seconds into unit string (e.g. "030s", "010m", "02h") */
static void format_timeunit_value(uint32_t seconds, TimeUnit_t u, char *out, size_t out_sz)
{
    if (!out)
        return;
    uint32_t v = from_seconds_to_unit_value(seconds, u);
    if (u == UNIT_HOUR)
    {
        snprintf(out, out_sz, "%02u%s", (unsigned int)v, TIME_UNIT_LABELS[u]);
    }
    else
    {
        snprintf(out, out_sz, "%03u%s", (unsigned int)v, TIME_UNIT_LABELS[u]);
    }
}

/* --- Config helpers --- */

static void load_working_cfg(void)
{
    /* get current runtime configuration; if fails, use defaults */
    if (SYS_MGR_GetConfig(&g_working_cfg) != E_OK)
    {
        extern const SysMgr_Config_t g_default_system_configuration;
        memcpy(&g_working_cfg, &g_default_system_configuration, sizeof(g_working_cfg));
        LOGW(TAG, "Failed to load config, using defaults");
    }
}

static void discard_working_cfg(void)
{
    load_working_cfg();
}

/* Validate + commit to runtime + flash */
static Status_t commit_working_cfg(void)
{
    if (SYS_MGR_ValidateConfig(&g_working_cfg) != E_OK)
    {
        lcd_write_if_changed("Cfg Invalid", "Not Saved");
        return E_NOK;
    }

    if (SYS_MGR_UpdateConfigRuntime(&g_working_cfg) != E_OK)
    {
        lcd_write_if_changed("Update Failed", "Check System");
        return E_NOK;
    }

    if (SYS_MGR_SaveConfigToFlash() != E_OK)
    {
        lcd_write_if_changed("Flash Save Err", "");
        return E_NOK;
    }
    lcd_write_if_changed("Config Saved", "Returning...");
    return E_OK;
}

/* ------------------------------------------------------------------------- */
/* Rendering: dashboards & menus                                             */
/* ------------------------------------------------------------------------- */

/* Dashboard wrapper (dispatch to specific page renderer) */
static void render_dashboard(void)
{
    switch (g_dash_page)
    {
    case DASH_PAGE_AVG:
        render_dash_avg();
        break;
    case DASH_PAGE_SENSOR:
        render_dash_sensor();
        break;
    case DASH_PAGE_ACTUATORS:
        render_dash_actuators();
        break;
    case DASH_PAGE_MODETIME:
        render_dash_mode_time();
        break;
    default:
        render_dash_avg();
        break;
    }
}

/* Dashboard: system average */
static void render_dash_avg(void)
{
    SYS_MGR_Mode_t mode;
    SysClock_Time_t ct;

    char l1[LCD_COLS + 1] = {0};
    char l2[LCD_COLS + 1] = {0};
    float avg_t = 0.0f;
    float avg_h = 0.0f;
    Status_t stt = TempHumCtrl_GetSystemAverageTemperature(&avg_t);
    Status_t sth = TempHumCtrl_GetSystemAverageHumidity(&avg_h);

    if (stt == E_OK && sth == E_OK)
    {
        snprintf(l1, sizeof(l1), "AvgT:%4.1fC H:%2.0f%%", avg_t, avg_h);
    }
    else if (stt == E_OK)
    {
        snprintf(l1, sizeof(l1), "AvgT:%4.1fC H:--%%", avg_t);
    }
    else
    {
        snprintf(l1, sizeof(l1), "AvgT: --.-  H:--%%");
    }

    /* show mode and simple time */
    ct = SysMgr_GetCurrentTime();
    
    if (SYS_MGR_GetMode(&mode) == E_OK)
    {
        const char *mstr = (mode == SYS_MGR_MODE_AUTOMATIC) ? "AUTO" : (mode == SYS_MGR_MODE_HYBRID) ? "HYBRID"
                                                                   : (mode == SYS_MGR_MODE_MANUAL)   ? "MANUAL"
                                                                                                     : "SAFE";
        snprintf(l2, sizeof(l2), "Mode:%-6s %02u:%02u", mstr, ct.hour, ct.minute);
    }
    else
    {
        snprintf(l2, sizeof(l2), "Mode:ERR --:--");
    }

    lcd_write_if_changed(l1, l2);
}

/* Dashboard: one sensor details (rotates across sensors) */
static void render_dash_sensor(void)
{
    char l1[LCD_COLS + 1] = {0};
    char l2[LCD_COLS + 1] = {0};
    uint8_t sensor_count = 0;

    if (TempHumCtrl_GetConfiguredSensorCount(&sensor_count) != E_OK || sensor_count == 0)
    {
        snprintf(l1, sizeof(l1), "No sensors");
        snprintf(l2, sizeof(l2), "Check config");
        lcd_write_if_changed(l1, l2);
        return;
    }

    /* sanitize index */
    if (g_sensor_index >= sensor_count)
        g_sensor_index = 0;

    float t = 0.0f, h = 0.0f;
    Status_t stt = TempHumCtrl_GetTemperature((TempHum_Sensor_ID_t)g_sensor_index, &t);
    Status_t sth = TempHumCtrl_GetHumidity((TempHum_Sensor_ID_t)g_sensor_index, &h);

    snprintf(l1, sizeof(l1), "S%u ", (unsigned int)(g_sensor_index + 1));
    if (stt == E_OK)
    {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "T:%4.1fC", t);
        strncat(l1, tmp, sizeof(l1) - strlen(l1) - 1);
    }
    else
    {
        strncat(l1, "T:--.-C", sizeof(l1) - strlen(l1) - 1);
    }

    if (sth == E_OK)
    {
        snprintf(l2, sizeof(l2), "H:%2.0f%%", h);
    }
    else if (sth == E_NOT_SUPPORTED)
    {
        snprintf(l2, sizeof(l2), "H:N/A");
    }
    else
    {
        snprintf(l2, sizeof(l2), "H:--%%");
    }

    lcd_write_if_changed(l1, l2);
}

/* Dashboard: show actuator states (counts or first few statuses) */
static void render_dash_actuators(void)
{
    char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    SYS_MGR_Actuator_States_t st;
    
    if (SYS_MGR_GetActuatorStates(&st) == E_OK)
    {
        snprintf(l1, sizeof(l1), "Fan:%s Heat:%s",
                 st.fans_active ? "ON " : "OFF",
                 st.heaters_active ? "ON " : "OFF");
        snprintf(l2, sizeof(l2), "Pump:%s Vent:%s",
                 st.pumps_active ? "ON " : "OFF",
                 st.vents_active ? "ON " : "OFF");
    }
    else
    {
        snprintf(l1, sizeof(l1), "Actuators N/A");
        snprintf(l2, sizeof(l2), "Check System");
    }
    lcd_write_if_changed(l1, l2);
}

/* Dashboard: mode and time concise (we show detailed in avg as well) */
static void render_dash_mode_time(void)
{
    char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    SYS_MGR_Mode_t mode;
    if (SYS_MGR_GetMode(&mode) == E_OK)
    {
        const char *mstr = (mode == SYS_MGR_MODE_AUTOMATIC) ? "AUTO" : (mode == SYS_MGR_MODE_HYBRID) ? "HYBRID"
                                                                   : (mode == SYS_MGR_MODE_MANUAL)   ? "MANUAL"
                                                                                                     : "SAFE";
        snprintf(l1, sizeof(l1), "Mode: %s", mstr);
    }
    else
    {
        snprintf(l1, sizeof(l1), "Mode:ERR");
    }
    SysClock_Time_t ct = SysMgr_GetCurrentTime();
    snprintf(l2, sizeof(l2), "Time %02u:%02u:%02u", ct.hour, ct.minute, ct.second);
    lcd_write_if_changed(l1, l2);
}

/* Menu root */
static void render_menu_root(void)
{
    lcd_write_if_changed("1:T 2:HU 3:F 4:V","5:P 6:H 7:L 8:P");
}

/* Global temp edit: show both fields; input shows replacing current editing field */
static void render_edit_temp_global(void)
{
    char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};

    if (g_editing_min)
    {
        snprintf(l1, sizeof(l1), "Set Tmin:");
        if (!input_is_empty())
        {
            snprintf(l2, sizeof(l2), "%s C", g_input_buf);
        }
        else
        {
            snprintf(l2, sizeof(l2), "%.1f C", g_working_cfg.global_temp_min);
        }
    }
    else
    {
        snprintf(l1, sizeof(l1), "Set Tmax:");
        if (!input_is_empty())
        {
            snprintf(l2, sizeof(l2), "%s C", g_input_buf);
        }
        else
        {
            snprintf(l2, sizeof(l2), "%.1f C", g_working_cfg.global_temp_max);
        }
    }

    lcd_write_if_changed(l1, l2);
}

/* Global hum edit */
static void render_edit_hum_global(void)
{
        char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};

    if (g_editing_min)
    {
        snprintf(l1, sizeof(l1), "Set Hmin:");
        if (!input_is_empty())
            snprintf(l2, sizeof(l2), "%s %%", g_input_buf);
        else
            snprintf(l2, sizeof(l2), "%.1f %%", g_working_cfg.global_hum_min);
    }
    else
    {
        snprintf(l1, sizeof(l1), "Set Hmax:");
        if (!input_is_empty())
            snprintf(l2, sizeof(l2), "%s %%", g_input_buf);
        else
            snprintf(l2, sizeof(l2), "%.1f %%", g_working_cfg.global_hum_max);
    }

    lcd_write_if_changed(l1, l2);
}

/* Actuator UI: select ID */
static void render_actuator_select_id(void)
{
        char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    const char *atype = (g_act_ctx.type == SYS_MGR_ACTUATOR_FANS) ? "Fan" : (g_act_ctx.type == SYS_MGR_ACTUATOR_HEATERS) ? "Heater"
                                                                        : (g_act_ctx.type == SYS_MGR_ACTUATOR_PUMPS)     ? "Pump"
                                                                        : (g_act_ctx.type == SYS_MGR_ACTUATOR_VENTS)     ? "Vent"
                                                                        : (g_act_ctx.type == SYS_MGR_ACTUATOR_LIGHTS)    ? "Light"
                                                                                                                         : "Act";

    uint16_t count = 0;
    switch (g_act_ctx.type)
    {
    case SYS_MGR_ACTUATOR_FANS:
        count = FAN_ID_COUNT;
        break;
    case SYS_MGR_ACTUATOR_HEATERS:
        count = HEATER_ID_COUNT;
        break;
    case SYS_MGR_ACTUATOR_PUMPS:
        count = PUMP_ID_COUNT;
        break;
    case SYS_MGR_ACTUATOR_VENTS:
        count = VEN_ID_COUNT;
        break;
    case SYS_MGR_ACTUATOR_LIGHTS:
        count = LIGHT_ID_COUNT;
        break;
    default:
        count = 0;
        break;
    }

    snprintf(l1, sizeof(l1), "%s Select ID", atype);
    snprintf(l2, sizeof(l2), "ID:%u of %u", (unsigned int)(g_act_ctx.id + 1), (unsigned int)count);
    lcd_write_if_changed(l1, l2);
}

/* Actuator mode (auto/manual) */
static void render_actuator_mode(void)
{
    char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    const char *atype = (g_act_ctx.type == SYS_MGR_ACTUATOR_FANS) ? "Fan" : (g_act_ctx.type == SYS_MGR_ACTUATOR_HEATERS) ? "Heater"
                                                                        : (g_act_ctx.type == SYS_MGR_ACTUATOR_PUMPS)     ? "Pump"
                                                                        : (g_act_ctx.type == SYS_MGR_ACTUATOR_VENTS)     ? "Vent"
                                                                                                                         : "Act";

    snprintf(l1, sizeof(l1), "%s%u Mode?", atype, (unsigned int)(g_act_ctx.id + 1));
    snprintf(l2, sizeof(l2), "1:Auto 2:Manual");
    lcd_write_if_changed(l1, l2);
}

/* Actuator manual ON/OFF edit screen */
static void render_actuator_manual_edit(void)
{
    char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    char onbuf[16] = {0}, offbuf[16] = {0};
    format_timeunit_value(g_act_ctx.on_seconds, g_act_ctx.unit_on, onbuf, sizeof(onbuf));
    format_timeunit_value(g_act_ctx.off_seconds, g_act_ctx.unit_off, offbuf, sizeof(offbuf));

    /* If user is typing, show input for active field */
    if (!input_is_empty())
    {
        if (g_field_pos == 0)
        {
            snprintf(l1, sizeof(l1), "ON:%s", g_input_buf);
            snprintf(l2, sizeof(l2), "OFF:%s", offbuf);
        }
        else
        {
            snprintf(l1, sizeof(l1), "ON:%s", onbuf);
            snprintf(l2, sizeof(l2), "OFF:%s", g_input_buf);
        }
    }
    else
    {
        snprintf(l1, sizeof(l1), "ON:%s", onbuf);
        snprintf(l2, sizeof(l2), "OFF:%s", offbuf);
    }

    lcd_write_if_changed(l1, l2);
}

/* Light edit (ON & OFF time on two lines) */
static void render_light_edit(void)
{
        char l1[LCD_COLS + 1] = {0}; 
    char l2[LCD_COLS + 1] = {0};
    snprintf(l1, sizeof(l1), "ON:%02u:%02u", g_light_ctx.on_h, g_light_ctx.on_m);
    snprintf(l2, sizeof(l2), "OFF:%02u:%02u", g_light_ctx.off_h, g_light_ctx.off_m);
    lcd_write_if_changed(l1, l2);
}

/* ------------------------------------------------------------------------- */
/* Event handling: central dispatchers for each UI state                      */
/* ------------------------------------------------------------------------- */

static void handle_event_main(const Keypad_Event_t *ev)
{
    if (!ev)
        return;

    if (ev->type == KEYPAD_EVT_HOLD && ev->button == KEYPAD_BTN_ENTER)
    {
        /* Enter configuration menu: load working copy */
        load_working_cfg();
        input_clear();
        g_editing_min = true;
        g_field_pos = 0;
        g_state = UI_STATE_MENU_ROOT;
        render_menu_root();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS)
    {
        if (ev->button == KEYPAD_BTN_LEFT)
        {
            g_dash_page = (g_dash_page == 0) ? (DASH_PAGE_COUNT - 1) : (DashPage_t)(g_dash_page - 1);
            render_dashboard();
        }
        else if (ev->button == KEYPAD_BTN_RIGHT)
        {
            g_dash_page = (DashPage_t)((g_dash_page + 1) % DASH_PAGE_COUNT);
            render_dashboard();
        }
        else if (ev->button == KEYPAD_BTN_UP)
        {
            /* on sensor page, go to previous sensor */
            if (g_dash_page == DASH_PAGE_SENSOR)
            {
                uint8_t sc = 0;
                if (TempHumCtrl_GetConfiguredSensorCount(&sc) == E_OK && sc > 0)
                {
                    g_sensor_index = (g_sensor_index == 0) ? (sc - 1) : (g_sensor_index - 1);
                    render_dash_sensor();
                }
            }
        }
        else if (ev->button == KEYPAD_BTN_DOWN)
        {
            if (g_dash_page == DASH_PAGE_SENSOR)
            {
                uint8_t sc = 0;
                if (TempHumCtrl_GetConfiguredSensorCount(&sc) == E_OK && sc > 0)
                {
                    g_sensor_index = (g_sensor_index + 1) % sc;
                    render_dash_sensor();
                }
            }
        }
    }
}

/**
 * @brief Handles keypad events when inside the root configuration menu.
 *
 * Key mapping (2×16 LCD):
 *   Line1: 1:T 2:HU 3:F 4:V
 *   Line2: 5:P 6:H 7:L 8:P
 *
 * 1 → Configure global temperature thresholds
 * 2 → Configure global humidity thresholds
 * 3 → Fan configuration (manual/auto + timers)
 * 4 → Vent configuration
 * 5 → Pump configuration
 * 6 → Heater configuration
 * 7 → Light schedule configuration
 * 8 → Per-sensor config (placeholder, not yet implemented)
 * BACK → return to main screen without saving
 * ENTER (hold) → reserved for future (unit toggle, etc.)
 */
static void handle_event_menu_root(const Keypad_Event_t *ev)
{
    if (!ev) return;
    if (ev->type != KEYPAD_EVT_PRESS) {
        /* only process presses in root */
        if (ev->type == KEYPAD_EVT_HOLD && ev->button == KEYPAD_BTN_BACK) {
            /* hold BACK = discard and return to main */
            discard_working_cfg();
            g_state = UI_STATE_MAIN_SCREEN;
            g_dash_page = DASH_PAGE_AVG;
            render_dashboard();
        }
        return;
    }

    /* Numeric selection map */
    switch (ev->button) {
        case KEYPAD_BTN_1: // Global Temp
            g_state = UI_STATE_EDIT_GLOBAL_TEMP;
            input_clear();
            g_editing_min = true;
            render_edit_temp_global();
            break;

        case KEYPAD_BTN_2:
            g_state = UI_STATE_EDIT_GLOBAL_HUM;
            input_clear();
            g_editing_min = true;
            render_edit_hum_global();
            break;

        case KEYPAD_BTN_3: // Fans
            g_act_ctx.type = SYS_MGR_ACTUATOR_FANS;
            g_act_ctx.id = 0;
            g_state = UI_STATE_CONFIG_FAN;
            input_clear();
            /* Prepopulate from working cfg */
            g_act_ctx.unit_on = UNIT_SEC; g_act_ctx.unit_off = UNIT_SEC;
            g_act_ctx.on_seconds = g_working_cfg.fans_cycle.on_time_sec;
            g_act_ctx.off_seconds = g_working_cfg.fans_cycle.off_time_sec;
            render_actuator_mode();

            break;

        case KEYPAD_BTN_4: // Vents
            g_act_ctx.type = SYS_MGR_ACTUATOR_VENTS;
            g_act_ctx.id = 0;
            g_state = UI_STATE_CONFIG_VENTS;
            input_clear();
            g_act_ctx.unit_on = UNIT_SEC; g_act_ctx.unit_off = UNIT_SEC;
            g_act_ctx.on_seconds = g_working_cfg.vents_cycle.on_time_sec;
            g_act_ctx.off_seconds = g_working_cfg.vents_cycle.off_time_sec;
            render_actuator_mode();
            break;

        case KEYPAD_BTN_5: // Pumps
            g_act_ctx.type = SYS_MGR_ACTUATOR_PUMPS;
            g_act_ctx.id = 0;
            g_state = UI_STATE_CONFIG_PUMP;
            input_clear();
            g_act_ctx.unit_on = UNIT_SEC; g_act_ctx.unit_off = UNIT_SEC;
            g_act_ctx.on_seconds = g_working_cfg.pumps_cycle.on_time_sec;
            g_act_ctx.off_seconds = g_working_cfg.pumps_cycle.off_time_sec;
            render_actuator_mode();
            break;

        case KEYPAD_BTN_6: // Heaters
            g_act_ctx.type = SYS_MGR_ACTUATOR_HEATERS;
            g_act_ctx.id = 0;
            g_state = UI_STATE_CONFIG_HEATER;
            input_clear();
            g_act_ctx.unit_on = UNIT_SEC; g_act_ctx.unit_off = UNIT_SEC;
            g_act_ctx.on_seconds = g_working_cfg.heaters_cycle.on_time_sec;
            g_act_ctx.off_seconds = g_working_cfg.heaters_cycle.off_time_sec;
            render_actuator_mode();
            break;

        case KEYPAD_BTN_7: // Lights
            /* Light schedule */
            g_state = UI_STATE_CONFIG_LIGHT;
            /* load existing */
            g_light_ctx.on_h = g_working_cfg.light_schedule.on_hour;
            g_light_ctx.on_m = g_working_cfg.light_schedule.on_min;
            g_light_ctx.off_h = g_working_cfg.light_schedule.off_hour;
            g_light_ctx.off_m = g_working_cfg.light_schedule.off_min;
            render_light_edit();
            break;
        case KEYPAD_BTN_8:
            /* Per-sensor placeholder */
            lcd_write_if_changed("Per-Sensor", "Not Implemented");
            /* short back = return to dashboard, keep changes in working copy */
            g_state = UI_STATE_MAIN_SCREEN;
            g_dash_page = DASH_PAGE_AVG;
            render_dashboard();
            break;
        case KEYPAD_BTN_9:
            /* Save & Exit immediately */
            if (commit_working_cfg() == E_OK) {
                g_state = UI_STATE_MAIN_SCREEN;
                g_dash_page = DASH_PAGE_AVG;
                render_dashboard();
            } else {
                /* Error displayed by commit_working_cfg */
            }
            break;
        case KEYPAD_BTN_BACK:
            /* short back = return to dashboard, keep changes in working copy */
            g_state = UI_STATE_MAIN_SCREEN;
            g_dash_page = DASH_PAGE_AVG;
            render_dashboard();
            break;
        default:
            // ignore other keys
            break;
    }
}

/* Edit global temperature: numeric input + accept */
static void handle_event_edit_temp(const Keypad_Event_t *ev)
{
    if (!ev)
        return;

    if (ev->type == KEYPAD_EVT_PRESS && (ev->button >= KEYPAD_BTN_0 && ev->button <= KEYPAD_BTN_9))
    {
        input_append_digit((char)('0' + (ev->button - KEYPAD_BTN_0)));
        render_edit_temp_global();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ERASE)
    {
        input_erase_last();
        render_edit_temp_global();
        return;
    }

    if (ev->type == KEYPAD_EVT_HOLD && ev->button == KEYPAD_BTN_BACK)
    {
        /* cancel editing, revert working cfg */
        discard_working_cfg();
        g_state = UI_STATE_MENU_ROOT;
        render_menu_root();
        input_clear();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_BACK)
    {
        /* short back = erase last digit already handled above */
        input_erase_last();
        render_edit_temp_global();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ENTER)
    {
        /* accept field */
        if (input_is_empty())
        {
            /* ignore empty enter */
            lcd_write_if_changed("Enter a value", "");
            return;
        }
        float v = strtof(g_input_buf, NULL);
        /* validation against UI limits */
        if (v < UI_TEMP_MIN_LIMIT_C || v > UI_TEMP_MAX_LIMIT_C)
        {
            lcd_write_if_changed("Invalid temp", "Range 20-60C");
            input_clear();
            return;
        }

        if (g_editing_min)
        {
            g_working_cfg.global_temp_min = v;
            g_editing_min = false;
            input_clear();
            render_edit_temp_global();
            return;
        }
        else
        {
            /* editing max — ensure > min */
            if (v <= g_working_cfg.global_temp_min)
            {
                lcd_write_if_changed("Invalid Tmax", "Must be > Tmin");
                input_clear();
                return;
            }
            g_working_cfg.global_temp_max = v;
            /* done — return to menu root */
            lcd_write_if_changed("Temp Updated", "");
            /* optionally commit immediately or wait for Save & Exit; we leave to user */
            g_state = UI_STATE_MENU_ROOT;
            render_menu_root();
            input_clear();
            g_editing_min = true;
            return;
        }
    }
}

/* Edit global humidity */
static void handle_event_edit_hum(const Keypad_Event_t *ev)
{
    if (!ev)
        return;

    if (ev->type == KEYPAD_EVT_PRESS && (ev->button >= KEYPAD_BTN_0 && ev->button <= KEYPAD_BTN_9))
    {
        input_append_digit((char)('0' + (ev->button - KEYPAD_BTN_0)));
        render_edit_hum_global();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ERASE)
    {
        input_erase_last();
        render_edit_hum_global();
        return;
    }

    if (ev->type == KEYPAD_EVT_HOLD && ev->button == KEYPAD_BTN_BACK)
    {
        discard_working_cfg();
        g_state = UI_STATE_MENU_ROOT;
        render_menu_root();
        input_clear();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ENTER)
    {
        if (input_is_empty())
        {
            lcd_write_if_changed("Enter a value", "");
            return;
        }
        float v = strtof(g_input_buf, NULL);
        if (v < UI_HUM_MIN_LIMIT_P || v > UI_HUM_MAX_LIMIT_P)
        {
            lcd_write_if_changed("Invalid hum", "Range 20-60%");
            input_clear();
            return;
        }

        if (g_editing_min)
        {
            g_working_cfg.global_hum_min = v;
            g_editing_min = false;
            input_clear();
            render_edit_hum_global();
            return;
        }
        else
        {
            if (v <= g_working_cfg.global_hum_min)
            {
                lcd_write_if_changed("Invalid Hmax", "Must be > Hmin");
                input_clear();
                return;
            }
            g_working_cfg.global_hum_max = v;
            lcd_write_if_changed("Hum Updated", "");
            g_state = UI_STATE_MENU_ROOT;
            render_menu_root();
            input_clear();
            g_editing_min = true;
            return;
        }
    }
}

/* Actuator flow:
   - initial state after selecting actuator type: show mode select (render_actuator_mode)
   - pressing '1' sets Auto (disable cycle in working config)
   - pressing '2' sets Manual -> enter manual editor where user edits ON and OFF durations
   - In manual editor:
       * numeric keys append digits for active field (ON or OFF)
       * LEFT/RIGHT short toggles active field
       * ENTER short accepts current field and moves to next or saves if both done
       * ENTER hold toggles unit for active field (s/m/h)
       * HOLD LEFT/HOLD RIGHT changes actuator id (prev/next)
       * BACK short = erase, BACK hold = cancel (discard working cfg & return)
*/
static void handle_event_actuator(const Keypad_Event_t *ev)
{
    if (!ev)
        return;

    /* if we are in mode select (we determine by input buffer empty and field_pos) */
    if (g_input_len == 0 && g_field_pos == 0 && ev->type == KEYPAD_EVT_PRESS &&
        (g_state == UI_STATE_CONFIG_FAN || g_state == UI_STATE_CONFIG_VENTS ||
         g_state == UI_STATE_CONFIG_PUMP || g_state == UI_STATE_CONFIG_HEATER))
    {
        /* interpret numeric keys 1=auto 2=manual */
        if (ev->button == KEYPAD_BTN_1)
        {
            /* Auto: disable cycle in working cfg for this actuator type */
            Actuator_Cycle_t *c = NULL;
            switch (g_act_ctx.type)
            {
            case SYS_MGR_ACTUATOR_FANS:
                c = &g_working_cfg.fans_cycle;
                break;
            case SYS_MGR_ACTUATOR_HEATERS:
                c = &g_working_cfg.heaters_cycle;
                break;
            case SYS_MGR_ACTUATOR_PUMPS:
                c = &g_working_cfg.pumps_cycle;
                break;
            case SYS_MGR_ACTUATOR_VENTS:
                c = &g_working_cfg.vents_cycle;
                break;
            default:
                break;
            }
            if (c)
            {
                c->enabled = false;
                lcd_write_if_changed("Set Auto", "Returning...");
            }
            g_state = UI_STATE_MENU_ROOT;
            render_menu_root();
            return;
        }
        else if (ev->button == KEYPAD_BTN_2)
        {
            /* Manual: enter manual edit screen */
            g_act_ctx.manual = true;
            g_field_pos = 0; /* on-time first */
            input_clear();
            /* prefill units and seconds if available else defaults */
            if (g_act_ctx.type == SYS_MGR_ACTUATOR_FANS)
            {
                g_act_ctx.on_seconds = g_working_cfg.fans_cycle.on_time_sec ? g_working_cfg.fans_cycle.on_time_sec : SYS_MGR_DEFAULT_CYCLE_ON_SEC;
                g_act_ctx.off_seconds = g_working_cfg.fans_cycle.off_time_sec ? g_working_cfg.fans_cycle.off_time_sec : SYS_MGR_DEFAULT_CYCLE_OFF_SEC;
            }
            else if (g_act_ctx.type == SYS_MGR_ACTUATOR_HEATERS)
            {
                g_act_ctx.on_seconds = g_working_cfg.heaters_cycle.on_time_sec ? g_working_cfg.heaters_cycle.on_time_sec : SYS_MGR_DEFAULT_CYCLE_ON_SEC;
                g_act_ctx.off_seconds = g_working_cfg.heaters_cycle.off_time_sec ? g_working_cfg.heaters_cycle.off_time_sec : SYS_MGR_DEFAULT_CYCLE_OFF_SEC;
            }
            else if (g_act_ctx.type == SYS_MGR_ACTUATOR_PUMPS)
            {
                g_act_ctx.on_seconds = g_working_cfg.pumps_cycle.on_time_sec ? g_working_cfg.pumps_cycle.on_time_sec : SYS_MGR_DEFAULT_CYCLE_ON_SEC;
                g_act_ctx.off_seconds = g_working_cfg.pumps_cycle.off_time_sec ? g_working_cfg.pumps_cycle.off_time_sec : SYS_MGR_DEFAULT_CYCLE_OFF_SEC;
            }
            else if (g_act_ctx.type == SYS_MGR_ACTUATOR_VENTS)
            {
                g_act_ctx.on_seconds = g_working_cfg.vents_cycle.on_time_sec ? g_working_cfg.vents_cycle.on_time_sec : SYS_MGR_DEFAULT_CYCLE_ON_SEC;
                g_act_ctx.off_seconds = g_working_cfg.vents_cycle.off_time_sec ? g_working_cfg.vents_cycle.off_time_sec : SYS_MGR_DEFAULT_CYCLE_OFF_SEC;
            }
            /* default units seconds */
            g_act_ctx.unit_on = UNIT_SEC;
            g_act_ctx.unit_off = UNIT_SEC;
            render_actuator_manual_edit();
            return;
        }
    }

    /* manual editing state */
    if (g_act_ctx.manual)
    {
        if (ev->type == KEYPAD_EVT_PRESS)
        {
            if (ev->button >= KEYPAD_BTN_0 && ev->button <= KEYPAD_BTN_9)
            {
                /* numeric input for active field */
                input_append_digit((char)('0' + (ev->button - KEYPAD_BTN_0)));
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_ERASE)
            {
                input_erase_last();
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_BACK)
            {
                /* short back = erase last digit; hold handled below */
                input_erase_last();
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_LEFT || ev->button == KEYPAD_BTN_RIGHT)
            {
                /* switch active field */
                g_field_pos = (g_field_pos == 0) ? 1 : 0;
                input_clear();
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_ENTER)
            {
                /* accept current field */
                if (!input_is_empty())
                {
                    uint32_t val = (uint32_t)atoi(g_input_buf);
                    bool ok = false;
                    uint32_t secs = 0;
                    if (g_field_pos == 0)
                    {
                        /* ON field */
                        if (g_act_ctx.unit_on == UNIT_SEC)
                        {
                            ok = (val >= 1 && val <= ACT_MAX_SEC);
                        }
                        else if (g_act_ctx.unit_on == UNIT_MIN)
                        {
                            ok = (val >= 1 && val <= ACT_MAX_MIN);
                        }
                        else
                        {
                            ok = (val >= 1 && val <= ACT_MAX_HOUR);
                        }
                        if (ok)
                        {
                            secs = to_seconds_from_unit(val, g_act_ctx.unit_on);
                            g_act_ctx.on_seconds = secs;
                        }
                    }
                    else
                    {
                        /* OFF field */
                        if (g_act_ctx.unit_off == UNIT_SEC)
                        {
                            ok = (val >= 1 && val <= ACT_MAX_SEC);
                        }
                        else if (g_act_ctx.unit_off == UNIT_MIN)
                        {
                            ok = (val >= 1 && val <= ACT_MAX_MIN);
                        }
                        else
                        {
                            ok = (val >= 1 && val <= ACT_MAX_HOUR);
                        }
                        if (ok)
                        {
                            secs = to_seconds_from_unit(val, g_act_ctx.unit_off);
                            g_act_ctx.off_seconds = secs;
                        }
                    }
                    if (!ok)
                    {
                        lcd_write_if_changed("Invalid value", "Check limits");
                        input_clear();
                        return;
                    }
                    input_clear();
                    /* if both fields filled (non-zero), save into working config for this actuator type */
                    if (g_act_ctx.on_seconds > 0 && g_act_ctx.off_seconds > 0)
                    {
                        Actuator_Cycle_t *c = NULL;
                        switch (g_act_ctx.type)
                        {
                        case SYS_MGR_ACTUATOR_FANS:
                            c = &g_working_cfg.fans_cycle;
                            break;
                        case SYS_MGR_ACTUATOR_HEATERS:
                            c = &g_working_cfg.heaters_cycle;
                            break;
                        case SYS_MGR_ACTUATOR_PUMPS:
                            c = &g_working_cfg.pumps_cycle;
                            break;
                        case SYS_MGR_ACTUATOR_VENTS:
                            c = &g_working_cfg.vents_cycle;
                            break;
                        default:
                            c = NULL;
                            break;
                        }
                        if (c)
                        {
                            c->enabled = true;
                            c->on_time_sec = g_act_ctx.on_seconds;
                            c->off_time_sec = g_act_ctx.off_seconds;
                        }
                        lcd_write_if_changed("Act Saved", "Returning...");
                        /* leave manual edit */
                        g_act_ctx.manual = false;
                        g_state = UI_STATE_MENU_ROOT;
                        render_menu_root();
                        return;
                    }
                    else
                    {
                        /* move to next field */
                        g_field_pos = (g_field_pos == 0) ? 1 : 0;
                        render_actuator_manual_edit();
                        return;
                    }
                }
                else
                {
                    /* nothing typed — move to next field or ignore */
                    g_field_pos = (g_field_pos == 0) ? 1 : 0;
                    render_actuator_manual_edit();
                    return;
                }
            }
        }
        else if (ev->type == KEYPAD_EVT_HOLD)
        {
            if (ev->button == KEYPAD_BTN_ENTER)
            {
                /* toggle unit for active field */
                if (g_field_pos == 0)
                    g_act_ctx.unit_on = (TimeUnit_t)((g_act_ctx.unit_on + 1) % UNIT_COUNT);
                else
                    g_act_ctx.unit_off = (TimeUnit_t)((g_act_ctx.unit_off + 1) % UNIT_COUNT);
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_RIGHT)
            {
                /* move to next actuator id */
                uint16_t cnt = 1;
                switch (g_act_ctx.type)
                {
                case SYS_MGR_ACTUATOR_FANS:
                    cnt = FAN_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_HEATERS:
                    cnt = HEATER_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_PUMPS:
                    cnt = PUMP_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_VENTS:
                    cnt = VEN_ID_COUNT;
                    break;
                default:
                    cnt = 1;
                    break;
                }
                if (cnt > 0)
                    g_act_ctx.id = (g_act_ctx.id + 1) % cnt;
                /* reload times from cfg into ctx */
                Actuator_Cycle_t *c = NULL;
                switch (g_act_ctx.type)
                {
                case SYS_MGR_ACTUATOR_FANS:
                    c = &g_working_cfg.fans_cycle;
                    break;
                case SYS_MGR_ACTUATOR_HEATERS:
                    c = &g_working_cfg.heaters_cycle;
                    break;
                case SYS_MGR_ACTUATOR_PUMPS:
                    c = &g_working_cfg.pumps_cycle;
                    break;
                case SYS_MGR_ACTUATOR_VENTS:
                    c = &g_working_cfg.vents_cycle;
                    break;
                default:
                    c = NULL;
                    break;
                }
                if (c)
                {
                    g_act_ctx.on_seconds = c->on_time_sec;
                    g_act_ctx.off_seconds = c->off_time_sec;
                }
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_LEFT)
            {
                uint16_t cnt = 1;
                switch (g_act_ctx.type)
                {
                case SYS_MGR_ACTUATOR_FANS:
                    cnt = FAN_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_HEATERS:
                    cnt = HEATER_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_PUMPS:
                    cnt = PUMP_ID_COUNT;
                    break;
                case SYS_MGR_ACTUATOR_VENTS:
                    cnt = VEN_ID_COUNT;
                    break;
                default:
                    cnt = 1;
                    break;
                }
                if (cnt > 0)
                    g_act_ctx.id = (g_act_ctx.id == 0) ? (cnt - 1) : (g_act_ctx.id - 1);
                Actuator_Cycle_t *c = NULL;
                switch (g_act_ctx.type)
                {
                case SYS_MGR_ACTUATOR_FANS:
                    c = &g_working_cfg.fans_cycle;
                    break;
                case SYS_MGR_ACTUATOR_HEATERS:
                    c = &g_working_cfg.heaters_cycle;
                    break;
                case SYS_MGR_ACTUATOR_PUMPS:
                    c = &g_working_cfg.pumps_cycle;
                    break;
                case SYS_MGR_ACTUATOR_VENTS:
                    c = &g_working_cfg.vents_cycle;
                    break;
                default:
                    c = NULL;
                    break;
                }
                if (c)
                {
                    g_act_ctx.on_seconds = c->on_time_sec;
                    g_act_ctx.off_seconds = c->off_time_sec;
                }
                render_actuator_manual_edit();
                return;
            }
            else if (ev->button == KEYPAD_BTN_BACK)
            {
                /* hold back = cancel entirely, discard changes */
                discard_working_cfg();
                g_act_ctx.manual = false;
                g_state = UI_STATE_MENU_ROOT;
                render_menu_root();
                return;
            }
        }
        return;
    }

    /* If not manual, we are either in id selection or mode selection */
    if (ev->type == KEYPAD_EVT_PRESS)
    {
        /* digits used to select id if we are in id selection submode; for simplicity user presses numeric to select id after selecting type */
        if (ev->button >= KEYPAD_BTN_0 && ev->button <= KEYPAD_BTN_9)
        {
            uint8_t dig = (uint8_t)(ev->button - KEYPAD_BTN_0);
            if (dig == 0)
                return; /* no zero id */
            uint16_t cnt = 1;
            switch (g_act_ctx.type)
            {
            case SYS_MGR_ACTUATOR_FANS:
                cnt = FAN_ID_COUNT;
                break;
            case SYS_MGR_ACTUATOR_HEATERS:
                cnt = HEATER_ID_COUNT;
                break;
            case SYS_MGR_ACTUATOR_PUMPS:
                cnt = PUMP_ID_COUNT;
                break;
            case SYS_MGR_ACTUATOR_VENTS:
                cnt = VEN_ID_COUNT;
                break;
            default:
                cnt = 1;
                break;
            }
            if (dig <= cnt)
            {
                g_act_ctx.id = (uint16_t)(dig - 1);
                /* After id chosen, show mode select */
                render_actuator_mode();
                return;
            }
        }
        else if (ev->button == KEYPAD_BTN_BACK)
        {
            /* short back returns to menu root */
            g_state = UI_STATE_MENU_ROOT;
            render_menu_root();
            return;
        }
    }

    /* else ignore */
}

/* Light schedule editing:
   - Input expects HHMM digits (or allow up to 4 digits to set H/M fields)
   - Simple model: user presses digits to fill ON hour/min and OFF hour/min sequentially
   - BACK short = erase last digit, BACK hold = cancel
   - ENTER short = confirm (validate not equal)
*/
static void handle_event_light(const Keypad_Event_t *ev)
{
    static uint8_t pos = 0;         /* 0:on_h tens,1:on_h ones/ or just field index 0..3 */
    static char tmp_field[5] = {0}; /* holds up to 4 digits as user types ON/ OFF */

    if (!ev)
        return;

    if (ev->type == KEYPAD_EVT_PRESS && ev->button >= KEYPAD_BTN_0 && ev->button <= KEYPAD_BTN_9)
    {
        /* append digit to current field (we use simple per-field editing) */
        if (g_input_len < INPUT_MAX_LEN)
        {
            input_append_digit((char)('0' + (ev->button - KEYPAD_BTN_0)));
        }
        render_light_edit();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ERASE)
    {
        input_erase_last();
        render_light_edit();
        return;
    }

    if (ev->type == KEYPAD_EVT_HOLD && ev->button == KEYPAD_BTN_BACK)
    {
        discard_working_cfg();
        g_state = UI_STATE_MENU_ROOT;
        render_menu_root();
        input_clear();
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_BACK)
    {
        /* short back: if editing field erase last; else return to menu root */
        if (!input_is_empty())
        {
            input_erase_last();
            render_light_edit();
        }
        else
        {
            g_state = UI_STATE_MENU_ROOT;
            render_menu_root();
        }
        return;
    }

    if (ev->type == KEYPAD_EVT_PRESS && ev->button == KEYPAD_BTN_ENTER)
    {
        /* commit: parse input buffer expecting HHMMHHMM? For simplicity, we accept prefilled ctx editing */
        /* Validate ON != OFF */
        if (g_light_ctx.on_h == g_light_ctx.off_h && g_light_ctx.on_m == g_light_ctx.off_m)
        {
            lcd_write_if_changed("Invalid times", "ON != OFF");
            return;
        }
        /* Save to working config */
        g_working_cfg.light_schedule.on_hour = g_light_ctx.on_h;
        g_working_cfg.light_schedule.on_min = g_light_ctx.on_m;
        g_working_cfg.light_schedule.off_hour = g_light_ctx.off_h;
        g_working_cfg.light_schedule.off_min = g_light_ctx.off_m;
        g_working_cfg.light_schedule.enabled = true;
        lcd_write_if_changed("Light Saved", "Returning...");
        g_state = UI_STATE_MENU_ROOT;
        render_menu_root();
        input_clear();
        return;
    }
}

/* ------------------------------------------------------------------------- */
/* Public API: init & main                                                   */
/* ------------------------------------------------------------------------- */

void UI_MGR_Init(void)
{
    /* initialize UI module state */
    memset(&g_working_cfg, 0, sizeof(g_working_cfg));
    load_working_cfg();
    input_clear();
    memset(s_last_l1, 0, sizeof(s_last_l1));
    memset(s_last_l2, 0, sizeof(s_last_l2));

    g_state = UI_STATE_MAIN_SCREEN;
    g_dash_page = DASH_PAGE_AVG;
    g_sensor_index = 0;
    g_last_display_ms = UI_MGR_GetTick();
    g_last_key_ms = UI_MGR_GetTick();

    render_dashboard();
    LOGI(TAG, "UI Manager initialized (final).");
}

void UI_MGR_MainFunction(void)
{
    uint32_t now = UI_MGR_GetTick();

    /* Poll keypad events (drain queue) */
    Keypad_Event_t ev;
    Status_t st;
    while ((st = KeypadMgr_GetEvent(&ev)) == E_OK)
    {
        g_last_key_ms = now;
        /* dispatch by current UI state */
        switch (g_state)
        {
        case UI_STATE_MAIN_SCREEN:
            handle_event_main(&ev);
            break;
        case UI_STATE_MENU_ROOT:
            handle_event_menu_root(&ev);
            break;
        case UI_STATE_EDIT_GLOBAL_TEMP:
            handle_event_edit_temp(&ev);
            break;
        case UI_STATE_EDIT_GLOBAL_HUM:
            handle_event_edit_hum(&ev);
            break;
        case UI_STATE_CONFIG_FAN:
        case UI_STATE_CONFIG_VENTS:
        case UI_STATE_CONFIG_PUMP:
        case UI_STATE_CONFIG_HEATER:
            handle_event_actuator(&ev);
            break;
        case UI_STATE_CONFIG_LIGHT:
            handle_event_light(&ev);
            break;
        default:
            break;
        }
    }
    if (st != E_OK && st != E_DATA_STALE)
    {
        LOGW(TAG, "KeypadMgr_GetEvent returned %d", st);
    }

    /* Manage auto-rotation for dashboard */
    if (g_state == UI_STATE_MAIN_SCREEN)
    {
        if ((now - g_last_display_ms) >= UI_SCREEN_ROTATE_MS)
        {
            g_last_display_ms = now;
            /* rotate dashboard page; prefer to show sensors if available */
            if (g_dash_page == DASH_PAGE_AVG)
            {
                uint8_t sc = 0;
                if (TempHumCtrl_GetConfiguredSensorCount(&sc) == E_OK && sc > 0)
                {
                    g_dash_page = DASH_PAGE_SENSOR;
                    g_sensor_index = (g_sensor_index + 1) % sc;
                }
                else
                {
                    g_dash_page = DASH_PAGE_ACTUATORS;
                }
            }
            else if (g_dash_page == DASH_PAGE_SENSOR)
            {
                g_dash_page = DASH_PAGE_ACTUATORS;
            }
            else if (g_dash_page == DASH_PAGE_ACTUATORS)
            {
                g_dash_page = DASH_PAGE_MODETIME;
            }
            else
            {
                g_dash_page = DASH_PAGE_AVG;
            }
            render_dashboard();
        }
    }
    else
    {
        /* Menu idle timeout: discard changes and return to dashboard */
        if ((now - g_last_key_ms) >= UI_MENU_TIMEOUT_MS)
        {
            LOGW(TAG, "Menu timeout — discarding changes");
            discard_working_cfg();
            g_state = UI_STATE_MAIN_SCREEN;
            g_dash_page = DASH_PAGE_AVG;
            render_dashboard();
        }
    }
}
