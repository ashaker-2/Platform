/**
 * @file ui_manager_cfg.c
 * @brief UI Manager configuration implementation
 * @version 2.3
 * @date 2025
 *
 * Provides helper functions for LCD display and system tick access.
 * Centralizes the actual implementation for functions declared
 * in ui_manager_cfg.h.
 */

#include "ui_manager_cfg.h"
#include "char_display.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

/* =============================================================================
 * DISPLAY HELPERS IMPLEMENTATION
 * ============================================================================= */

void ui_display_message(const char *line1, const char *line2)
{
    HAL_CharDisplay_ClearDisplay();

    HAL_CharDisplay_SetCursor(0, 0);
    if (line1)
    {
        HAL_CharDisplay_WriteString(line1);
    }

    HAL_CharDisplay_SetCursor(1, 0);
    if (line2)
    {
        HAL_CharDisplay_WriteString(line2);
    }
}

void ui_display_numeric_input(const char *label, const char *input)
{
    HAL_CharDisplay_ClearDisplay();

    HAL_CharDisplay_SetCursor(0, 0);
    if (label)
    {
        HAL_CharDisplay_WriteString(label);
    }

    HAL_CharDisplay_SetCursor(1, 0);
    if (input)
    {
        HAL_CharDisplay_WriteString(input);
    }
}

void ui_display_clear(void)
{
    HAL_CharDisplay_ClearDisplay();
}

/* =============================================================================
 * TICK SOURCE
 * ============================================================================= */

uint32_t UI_MGR_GetTick(void)
{
    /* Convert FreeRTOS ticks to milliseconds */
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}
