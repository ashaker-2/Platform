/**
 * @file ui_manager_cfg.h
 * @brief Configuration constants for the UI Manager
 * @version 2.2
 * @date 2025
 *
 * Centralized UI configuration (timings, sizes) moved out of ui_manager.c.
 * This file intentionally defines the exact macros used by ui_manager.c so
 * the implementation file needs only to include this header.
 */

#ifndef UI_MANAGER_CFG_H
#define UI_MANAGER_CFG_H

#include <stdint.h>

/** Screen rotation period in milliseconds */
#define UI_SCREEN_ROTATE_MS     3000   /**< Time to rotate main screens (ms) */

/** Menu timeout in milliseconds (auto-exit) */
#define UI_MENU_TIMEOUT_MS      60000  /**< Inactivity timeout for menu (ms) */

/** Maximum characters for numeric input buffer in ui_manager.c */
#define UI_MAX_INPUT_LEN        8      /**< Max characters for numeric input */

/** Main function execution period in milliseconds (used by ui_manager.c) */
#define UI_MGR_MAIN_PERIOD_MS   100

/** LCD display dimensions (used in UI header types) */
#define UI_LCD_ROWS             2
#define UI_LCD_COLS             16

void ui_display_message(const char *line1, const char *line2);
void ui_display_numeric_input(const char *label, const char *input);
void ui_display_clear();
uint32_t UI_MGR_GetTick(void);

#endif /* UI_MANAGER_CFG_H */