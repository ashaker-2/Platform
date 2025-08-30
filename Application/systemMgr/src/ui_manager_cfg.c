/**
 * @file ui_manager_cfg.c
 * @brief UI Manager configuration implementation
 * @version 1.0
 * @date 2025
 *
 * This module exists so we can later move configuration data here (runtime
 * read/write, persistent settings, etc.). For now the macros live in the
 * header and this .c is a placeholder.
 */

#include "ui_manager_cfg.h"
#include "char_display.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief Clears the entire LCD and displays two lines of text.
 *
 * @param line1 The string to display on the first line.
 * @param line2 The string to display on the second line.
 */
void ui_display_message(const char *line1, const char *line2)
{
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_SetCursor(0, 0);
    HAL_CharDisplay_WriteString(line1);
    HAL_CharDisplay_SetCursor(1, 0);
    HAL_CharDisplay_WriteString(line2);
}

/**
 * @brief Displays a label and a numeric input string on the LCD.
 *
 * @param label The label string for the input field.
 * @param input The string representing the numeric input value.
 */
void ui_display_numeric_input(const char *label, const char *input)
{
    HAL_CharDisplay_ClearDisplay();
    HAL_CharDisplay_SetCursor(0, 0);
    HAL_CharDisplay_WriteString(label);
    HAL_CharDisplay_SetCursor(1, 0);
    HAL_CharDisplay_WriteString(input);
}

/**
 * @brief Clears the entire LCD.
 */
void ui_display_clear()
{
    HAL_CharDisplay_ClearDisplay();
}

/**
 * @brief A simple hardware abstraction layer (HAL) function to get the
 * current system tick count in milliseconds.
 *
 * @return The current tick count.
 */
uint32_t UI_MGR_GetTick(void)
{
    // return (uint32_t)(millis());
    return (uint32_t)1000;
}

/* Intentionally empty for now. Configuration values are macros in the header.
 * If you'd like runtime-configurable settings (Flash-backed), we can add
 * getters/setters here and persist them with FlashMgr.
 */