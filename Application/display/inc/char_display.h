// app/inc/character_display.h

#ifndef CHARACTER_DISPLAY_H
#define CHARACTER_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"  // For E_OK/E_NOK
#include "hal_i2c.h"   // For ECUAL_I2C_ID_t

/**
 * @brief User-friendly IDs for configured Character Display instances.
 */
typedef enum CHARACTER_DISPLAY_ID_t {
    CHARACTER_DISPLAY_MAIN_STATUS = 0, ///< Main system status display (e.g., 16x2 LCD)
    CHARACTER_DISPLAY_ALARM_PANEL,     ///< Alarm system display (e.g., 20x4 LCD)
    CHARACTER_DISPLAY_TOTAL_UNITS      ///< Keep this last to get the count
} CHARACTER_DISPLAY_ID_t;

/**
 * @brief Structure to hold the configuration for a single Character Display unit.
 */
typedef struct CHARACTER_DISPLAY_Config_t {
    ECUAL_I2C_ID_t      i2c_id;      ///< The ECUAL I2C bus ID connected to the display.
    uint8_t             i2c_address; ///< The I2C address of the PCF8574 expander (common: 0x27 or 0x3F).
    uint8_t             num_columns; ///< Number of display columns (e.g., 16, 20).
    uint8_t             num_rows;    ///< Number of display rows (e.g., 2, 4).
    bool                has_backlight; ///< True if the display has a controllable backlight.
    bool                backlight_active_high; ///< True if HIGH turns backlight ON, false if LOW.
} CHARACTER_DISPLAY_Config_t;

/**
 * @brief Initializes all Character Display units based on their configurations.
 * This function powers up the display, clears it, and turns on the backlight (if configured).
 * @return E_OK if all displays are initialized successfully, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_Init(void);

/**
 * @brief Clears the entire display of a specific unit and sets the cursor to home (0,0).
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ID_t unit_id);

/**
 * @brief Sets the cursor position on the display.
 * The top-left corner is (0,0). Coordinates are 0-indexed.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @param col The column to set the cursor to.
 * @param row The row to set the cursor to.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ID_t unit_id, uint8_t col, uint8_t row);

/**
 * @brief Prints a null-terminated string to the display at the current cursor position.
 * Text wraps to the next line if it exceeds the current line.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @param str The string to print.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ID_t unit_id, const char *str);

/**
 * @brief Prints a single character to the display at the current cursor position.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @param c The character to print.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_PrintChar(CHARACTER_DISPLAY_ID_t unit_id, char c);

/**
 * @brief Turns the backlight of a specific display unit ON.
 * This function only has an effect if 'has_backlight' is true in the configuration.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_BacklightOn(CHARACTER_DISPLAY_ID_t unit_id);

/**
 * @brief Turns the backlight of a specific display unit OFF.
 * This function only has an effect if 'has_backlight' is true in the configuration.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @return E_OK if successful, E_NOK otherwise.
 */
uint8_t CHARACTER_DISPLAY_BacklightOff(CHARACTER_DISPLAY_ID_t unit_id);

#endif /* CHARACTER_DISPLAY_H */
