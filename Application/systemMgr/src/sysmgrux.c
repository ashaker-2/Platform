\
/* sysmgrux.c - simplified UX separated from SysMgr
 *
 * Provides:
 *  - rotating main screens (avg, per-sensor, actuators)
 *  - config menu (# to enter)
 *  - edit global temp/hum thresholds and toggle sets
 *  - commit to SysMgr on save/timeout
 */

#include "sysmgrux.h"
#include "sys_mgr.h"
#include "temphumctrl.h"
#include "keypad_mgr.h"
#include "char_display.h"
#include "logger.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SCREEN_ROTATE_MS 3000
#define MENU_TIMEOUT_MS  60000

typedef enum {
    UX_MAIN = 0,
    UX_MENU,
    UX_EDIT_TEMP,
    UX_EDIT_HUM,
    UX_EDIT_SETS,
    UX_SAVE_EXIT
} UX_State_t;

static UX_State_t ux_state;
static uint32_t ux_timer_ms;
static uint32_t ux_inactivity_ms;
static uint8_t ux_screen_idx;
static SysMgr_Config_t ux_work_cfg;
static char ux_input[8];
static uint8_t ux_in_len;

static void ux_clear_input(void)
{
    ux_in_len=0; ux_input[0]='\\0'; 
}
static void ux_append_digit(char d)
{
    if (ux_in_len+1 < sizeof(ux_input)) 
    { 
        ux_input[ux_in_len++]=d; ux_input[ux_in_len]='\\0'; 
    } 
}

static void lcd_clear(void)
{
    HAL_CharDisplay_ClearDisplay(); 
    HAL_CharDisplay_Home(); 
}

static void show_main_avg(void)
{
    float t=0,h=0;
    TempHumCtrl_GetSystemAverageTemperature(&t);
    TempHumCtrl_GetSystemAverageHumidity(&h);
    char buf[17];
    lcd_clear();
    snprintf(buf, sizeof(buf), "AvgT:%2.0fC H:%2.0f%%", t, h);
    HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString(buf);
    HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(" #:Menu");
}

static void show_main_sensors(void)
{
    lcd_clear();
    char line[17];
    for (int i=0;i<2;i++){
        int idx = i;
        float t=0,h=0;
        TempHumCtrl_GetTemperature((TempHum_Sensor_ID_t)idx, &t);
        Status_t r = TempHumCtrl_GetHumidity((TempHum_Sensor_ID_t)idx, &h);
        if (r==E_OK) snprintf(line, sizeof(line), "S%d T:%2.0f H:%2.0f", idx+1, t, h);
        else snprintf(line, sizeof(line), "S%d T:%2.0f H:--", idx+1, t);
        HAL_CharDisplay_SetCursor(i,0); HAL_CharDisplay_WriteString(line);
    }
}

static void show_main_actuators(void)
{
    SYS_MGR_ActuatorState_t st;
    SYS_MGR_GetActuatorStates(&st);
    char buf[17];
    lcd_clear();
    snprintf(buf, sizeof(buf), "Fan:%s He:%s", st.fan_any_on?"ON":"OFF", st.heater_on?"ON":"OFF");
    HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString(buf);
    snprintf(buf, sizeof(buf), "Ven:%s Pmp:%s", st.ven_any_on?"ON":"OFF", st.pump_on?"ON":"OFF");
    HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(buf);
}

static void rotate_screens(uint32_t tick_ms)
{
    ux_timer_ms += tick_ms;
    if (ux_timer_ms < SCREEN_ROTATE_MS) return;
    ux_timer_ms = 0;
    ux_screen_idx = (ux_screen_idx + 1) % 3;
    switch (ux_screen_idx){
        case 0: show_main_avg(); break;
        case 1: show_main_sensors(); break;
        case 2: show_main_actuators(); break;
    }
}

static bool fetch_key(char *out)
{
    Keypad_Event_t ev;
    if (KeypadMgr_GetLastEvent(&ev) != E_OK) return false;
    *out = ev.key_char;
    return true;
}

/* Menu helpers */
static void show_menu_root(void)
{
    lcd_clear();
    HAL_CharDisplay_SetCursor(0,0); 
    HAL_CharDisplay_WriteString("1:Tmp 2:Hum 3:Sets");
    HAL_CharDisplay_SetCursor(1,0); 
    HAL_CharDisplay_WriteString("9:Save *:Back");
}

static void enter_edit_temp(void)
{
    ux_state = UX_EDIT_TEMP;
    ux_clear_input();
    lcd_clear();
    HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Set Temp Min");
    HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("val>");
}

static void handle_edit_temp_char(char k)
{
    if (isdigit((unsigned char)k)) { ux_append_digit(k); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(ux_input); return; }
    if (k=='#') {
        int v = atoi(ux_input); if (v<0) v=0; if (v>99) v=99;
        ux_work_cfg.global.temp_min_c = (float)v;
        ux_clear_input();
        HAL_CharDisplay_ClearDisplay();
        HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Set Temp Max");
        HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("val>");
        ux_state = UX_EDIT_TEMP + 10; /* substate for max */
        return;
    }
    if (k=='*'){ ux_state = UX_MENU; show_menu_root(); }
}

static void handle_edit_temp_max_char(char k)
{
    if (isdigit((unsigned char)k)) { ux_append_digit(k); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(ux_input); return; }
    if (k=='#') {
        int v = atoi(ux_input); if (v<0) v=0; if (v>99) v=99;
        ux_work_cfg.global.temp_max_c = (float)v;
        ux_state = UX_MENU; ux_clear_input(); show_menu_root();
        return;
    }
    if (k=='*'){ ux_state = UX_MENU; show_menu_root(); }
}

static void enter_edit_hum(void)
{
    ux_state = UX_EDIT_HUM;
    ux_clear_input();
    lcd_clear();
    HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Set Hum Min");
    HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("val>");
}

static void handle_edit_hum_min(char k)
{
    if (isdigit((unsigned char)k)) { ux_append_digit(k); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(ux_input); return; }
    if (k=='#') {
        int v = atoi(ux_input); if (v<0) v=0; if (v>100) v=100;
        ux_work_cfg.global.hum_min_p = (float)v;
        ux_clear_input();
        HAL_CharDisplay_ClearDisplay();
        HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Set Hum Max");
        HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("val>");
        ux_state = UX_EDIT_HUM + 10;
        return;
    }
    if (k=='*'){ ux_state = UX_MENU; show_menu_root(); }
}

static void handle_edit_hum_max(char k)
{
    if (isdigit((unsigned char)k)) { ux_append_digit(k); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(ux_input); return; }
    if (k=='#') {
        int v = atoi(ux_input); if (v<0) v=0; if (v>100) v=100;
        ux_work_cfg.global.hum_max_p = (float)v;
        ux_state = UX_MENU; ux_clear_input(); show_menu_root();
        return;
    }
    if (k=='*'){ ux_state = UX_MENU; show_menu_root(); }
}

static void enter_edit_sets(void)
{
    ux_state = UX_EDIT_SETS;
    ux_clear_input();
    lcd_clear();
    HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Toggle set idx");
    HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("idx>");
}

static void handle_edit_sets_char(char k)
{
    if (isdigit((unsigned char)k)) { ux_append_digit(k); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString(ux_input); return; }
    if (k=='#') {
        int idx = atoi(ux_input);
        if (idx >=0 && idx < TEMPHUM_SENSOR_ID_COUNT) {
            ux_work_cfg.sets[idx].enabled = !ux_work_cfg.sets[idx].enabled;
            char buf[17];
            snprintf(buf,sizeof(buf),"Set%d %s", idx, ux_work_cfg.sets[idx].enabled?"EN":"DIS");
            lcd_clear(); HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString(buf);
            HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("#=More *=Back");
        } else {
            lcd_clear(); HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Invalid idx");
            HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("*=Back");
        }
        ux_clear_input();
        return;
    }
    if (k=='*'){ ux_state = UX_MENU; show_menu_root(); }
}

static void commit_and_save(void)
{
    /* update SysMgr runtime config and save to flash */
    if (SYS_MGR_UpdateConfigRuntime(&ux_work_cfg) == E_OK) {
        SYS_MGR_SaveConfigToFlash();
        lcd_clear(); HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Saved"); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("#=OK");
    } else {
        lcd_clear(); HAL_CharDisplay_SetCursor(0,0); HAL_CharDisplay_WriteString("Save Failed"); HAL_CharDisplay_SetCursor(1,0); HAL_CharDisplay_WriteString("*=Back");
    }
    ux_state = UX_MAIN;
    ux_timer_ms = 0;
}

Status_t SysMgrUX_Init(void)
{
    memset(&ux_work_cfg,0,sizeof(ux_work_cfg));
    SYS_MGR_GetConfig(&ux_work_cfg);
    ux_state = UX_MAIN;
    ux_timer_ms = 0;
    ux_inactivity_ms = 0;
    ux_screen_idx = 0;
    ux_clear_input();
    show_main_avg();
    return E_OK;
}

void SysMgrUX_MainFunction(uint32_t tick_ms)
{
    ux_inactivity_ms += tick_ms;
    char k;
    bool key = fetch_key(&k);
    if (key) ux_inactivity_ms = 0;

    if (ux_state == UX_MAIN) {
        rotate_screens(tick_ms);
        if (key && k == '#') {
            ux_state = UX_MENU;
            show_menu_root();
        }
        return;
    }

    if (ux_inactivity_ms >= MENU_TIMEOUT_MS) {
        /* auto-save current work and exit */
        SYS_MGR_UpdateConfigRuntime(&ux_work_cfg);
        SYS_MGR_SaveConfigToFlash();
        ux_state = UX_MAIN;
        show_main_avg();
        ux_inactivity_ms = 0;
        return;
    }

    /* handle keys based on state */
    if (!key) return;
    switch (ux_state) {
        case UX_MENU:
            if (k=='1') enter_edit_temp();
            else if (k=='2') enter_edit_hum();
            else if (k=='3') enter_edit_sets();
            else if (k=='9') commit_and_save();
            else if (k=='*') { ux_state = UX_MAIN; show_main_avg(); }
            break;
        case UX_EDIT_TEMP:
            handle_edit_temp_char(k);
            break;
        case UX_EDIT_TEMP + 10:
            handle_edit_temp_max_char(k);
            break;
        case UX_EDIT_HUM:
            handle_edit_hum_min(k);
            break;
        case UX_EDIT_HUM + 10:
            handle_edit_hum_max(k);
            break;
        case UX_EDIT_SETS:
            handle_edit_sets_char(k);
            break;
        default:
            break;
    }
}
