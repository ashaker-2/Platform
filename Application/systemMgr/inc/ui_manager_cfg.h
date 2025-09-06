/**
 * @file ui_manager_cfg.h
 * @brief Configuration for the UI Manager
 *
 * Defines screen size, timings, buffer sizes, and UI behavior constants.
 */

#ifndef UI_MANAGER_CFG_H
#define UI_MANAGER_CFG_H

#include <stdint.h>
#include <stdbool.h>

/* LCD size */
#define UI_LCD_ROWS              2
#define UI_LCD_COLS              16

/* Periods & timeouts */
#define UI_MGR_MAIN_PERIOD_MS    100   /* call period */
#define UI_SCREEN_ROTATE_MS      5000  /* auto-rotate dashboard screen */
#define UI_MENU_TIMEOUT_MS       15000 /* auto-exit menu if idle */

/* Input buffer size for numeric entry */
#define UI_MAX_INPUT_LEN         4

/* Temperature & humidity limits (user editable ranges) */
#define UI_TEMP_MIN_LIMIT_C      20.0f
#define UI_TEMP_MAX_LIMIT_C      60.0f
#define UI_HUM_MIN_LIMIT_P       20.0f
#define UI_HUM_MAX_LIMIT_P       60.0f

/* Actuator cycle constraints */
#define UI_ACT_ON_MAX_SEC        999
#define UI_ACT_ON_MAX_MIN        720
#define UI_ACT_ON_MAX_HR         12

/* Light schedule constraints */
#define UI_LIGHT_HOUR_MAX        23
#define UI_LIGHT_MINUTE_MAX      59

#endif /* UI_MANAGER_CFG_H */
