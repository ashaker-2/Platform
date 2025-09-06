/**
 * @file ui_manager_cfg.h
 * @brief Configuration constants for the UI Manager
 * @version 2.3
 * @date 2025
 *
 * Centralized configuration for the UI Manager.
 * - LCD dimensions
 * - Timing constants (screen rotation, menu timeout, main loop period)
 * - Input buffer sizes
 * - Validation limits for thresholds and timers
 *
 * Implementation note:
 * - The implementation functions are provided in ui_manager_cfg.c
 */

#ifndef UI_MANAGER_CFG_H
#define UI_MANAGER_CFG_H

#include <stdint.h>
#include <stdbool.h>

/* =============================================================================
 * LCD CONFIGURATION
 * ============================================================================= */

/** LCD display dimensions (2x16 character LCD) */
#define UI_LCD_ROWS 2
#define UI_LCD_COLS 16

/* =============================================================================
 * TIMING CONFIGURATION
 * ============================================================================= */

/** Main function execution period (ms) */
#define UI_MGR_MAIN_PERIOD_MS 100

/** Screen rotation interval (ms) */
#define UI_SCREEN_ROTATE_MS 3000

/** Menu inactivity timeout (ms) */
#define UI_MENU_TIMEOUT_MS 60000

/* =============================================================================
 * INPUT CONFIGURATION
 * ============================================================================= */

/** Maximum characters for numeric input buffer */
#define UI_MAX_INPUT_LEN 8

/* =============================================================================
 * THRESHOLD LIMITS
 * ============================================================================= */

/** Allowed global temperature range (Celsius) */
#define UI_TEMP_MIN_LIMIT_C 20.0f
#define UI_TEMP_MAX_LIMIT_C 60.0f

/** Allowed global humidity range (%) */
#define UI_HUM_MIN_LIMIT_P 20.0f
#define UI_HUM_MAX_LIMIT_P 60.0f

/** Maximum seconds allowed for actuator cycles */
#define UI_ACTUATOR_MAX_SEC 999

/** Maximum minutes allowed for actuator cycles */
#define UI_ACTUATOR_MAX_MIN 720

/** Maximum hours allowed for actuator cycles */
#define UI_ACTUATOR_MAX_HOUR 12

/* =============================================================================
 * LIGHT SCHEDULE LIMITS
 * ============================================================================= */

/** Light ON/OFF schedule limits */
#define UI_LIGHT_MAX_HOUR 23
#define UI_LIGHT_MAX_MIN 59

/* =============================================================================
 * DISPLAY HELPERS
 * ============================================================================= */

/**
 * @brief Display two lines of text on the LCD.
 *
 * Clears the LCD, sets cursor, and writes both lines.
 */
void ui_display_message(const char *line1, const char *line2);

/**
 * @brief Display numeric input with a label.
 *
 * Clears the LCD, writes label on line 1 and input string on line 2.
 */
void ui_display_numeric_input(const char *label, const char *input);

/**
 * @brief Clears the LCD display.
 */
void ui_display_clear(void);

/**
 * @brief Get current system tick (ms).
 *
 * Implementation uses FreeRTOS tick: xTaskGetTickCount() * portTICK_PERIOD_MS.
 */
uint32_t UI_MGR_GetTick(void);

#endif /* UI_MANAGER_CFG_H */
