/* ============================================================================
 * SOURCE FILE: char_display.h
 * ============================================================================*/
/**
 * @file char_display.h
 * @brief Public API for the character display module.
 *
 * This header provides functions to initialize, control, and write data to
 * a character LCD display connected in 4-bit mode.
 */

#ifndef CHAR_DISPLAY_H
#define CHAR_DISPLAY_H

#include <stdint.h>     // For uint8_t
#include <stdbool.h>    // For bool
#include "esp_err.h"    // For esp_err_t

/**
 * @brief Initializes the character display module.
 *
 * This function configures the necessary GPIO pins and sends the
 * initialization commands to the LCD to set it up for 4-bit mode,
 * 2 lines, and clears the display.
 *
 * @return ESP_OK on success, or an error code if GPIO configuration fails.
 */
esp_err_t HAL_CharDisplay_Init(void);

/**
 * @brief Writes a single character to the current cursor position on the display.
 *
 * @param c The character to write.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_WriteChar(char c);

/**
 * @brief Writes a null-terminated string to the display, starting from the current cursor position.
 *
 * @param str The string to write.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_WriteString(const char *str);

/**
 * @brief Sets the cursor position on the display.
 *
 * @param col The column number (0-indexed, typically 0-15 or 0-19 depending on display size).
 * @param row The row number (0-indexed, typically 0 for first line, 1 for second).
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_SetCursor(uint8_t col, uint8_t row);

/**
 * @brief Clears the entire display and sets the cursor to home position (0,0).
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_ClearDisplay(void);

/**
 * @brief Moves the cursor to the home position (0,0) without clearing the display.
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_Home(void);

/**
 * @brief Controls the overall display visibility.
 *
 * @param enable True to turn the display on, false to turn it off.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_EnableDisplay(bool enable);

/**
 * @brief Controls the visibility of the blinking block cursor.
 *
 * @param enable True to enable the blinking cursor, false to disable.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_EnableBlink(bool enable);

/**
 * @brief Controls the visibility of the underline cursor.
 *
 * @param enable True to enable the underline cursor, false to disable.
 * @return ESP_OK on success, or an error code.
 */
esp_err_t HAL_CharDisplay_EnableCursor(bool enable);

#endif /* CHAR_DISPLAY_H */
