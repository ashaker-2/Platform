// app/src/character_display.c

#include "character_display.h"
#include "character_display_config.h"
#include "ecual_i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // For vTaskDelay
#include "freertos/task.h"    // For vTaskDelay

// HD44780 LCD commands
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORDISPLAYSHIFT  0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// Flags for display entry mode
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYLEFT           0x02

// Flags for display on/off control
#define LCD_DISPLAYON           0x04
#define LCD_CURSORON            0x02
#define LCD_BLINKON             0x01

// Flags for display/cursor shift
#define LCD_MOVEDISPLAY         0x08
#define LCD_MOVERIGHT           0x04

// Flags for function set
#define LCD_8BITMODE            0x10
#define LCD_2LINE               0x08
#define LCD_5x10DOTS            0x04

// PCF8574 pin mapping (common for most I2C LCD backpacks)
#define RS_BIT                  0x01 // P0
#define RW_BIT                  0x02 // P1 (often tied to GND, so always write)
#define EN_BIT                  0x04 // P2
#define BL_BIT                  0x08 // P3 (Backlight, sometimes P7) - this is configurable now
#define D4_BIT                  0x10 // P4
#define D5_BIT                  0x20 // P5
#define D6_BIT                  0x40 // P6
#define D7_BIT                  0x80 // P7

static const char *TAG = "CHAR_DISPLAY";

// Internal state for each display
typedef struct {
    uint8_t current_col;
    uint8_t current_row;
    uint8_t display_function_flags; // Stores 2-line mode, 5x8 dots, 4-bit/8-bit mode
    uint8_t display_control_flags;  // Stores display on/off, cursor on/off, blink on/off
    uint8_t backlight_state_mask;   // Stores the current backlight state (BL_BIT or 0)
} CHARACTER_DISPLAY_State_t;

static CHARACTER_DISPLAY_State_t display_states[CHARACTER_DISPLAY_TOTAL_UNITS];

// Row offsets for 4-line displays (common mapping)
static const uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

/**
 * @brief Helper function to get the configuration for a specific display ID.
 * @param unit_id The CHARACTER_DISPLAY_ID_t of the display unit.
 * @return Pointer to the configuration struct, or NULL if ID is invalid.
 */
static const CHARACTER_DISPLAY_Config_t* display_get_config(CHARACTER_DISPLAY_ID_t unit_id) {
    if (unit_id >= CHARACTER_DISPLAY_TOTAL_UNITS) {
        ESP_LOGE(TAG, "Invalid Display ID: %u", unit_id);
        return NULL;
    }
    return &character_display_configurations[unit_id];
}

/**
 * @brief Sends a nibble (4 bits) to the LCD via I2C PCF8574.
 * The nibble is sent on D4-D7.
 * @param unit_id The display unit ID.
 * @param nibble The 4-bit data to send (upper 4 bits of a byte).
 * @param rs_mode RS_BIT (for data) or 0 (for command).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
static uint8_t lcd_send_nibble(CHARACTER_DISPLAY_ID_t unit_id, uint8_t nibble, uint8_t rs_mode) {
    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];

    // Prepare the byte to send to PCF8574
    // nibble is already shifted to D4-D7 position (e.g., 0x10, 0x20, 0x40, 0x80)
    // rs_mode is 0x01 (RS high for data) or 0x00 (RS low for command)
    uint8_t data = nibble | rs_mode | state->backlight_state_mask;

    // Send with EN high
    if (ECUAL_I2C_Master_Write(cfg->i2c_id, cfg->i2c_address, &data, 1) != ECUAL_OK) {
        return APP_ERROR;
    }
    vTaskDelay(pdMS_TO_TICKS(1)); // Small delay for pulse width

    // Pulse EN low
    data &= ~EN_BIT; // Clear EN bit
    if (ECUAL_I2C_Master_Write(cfg->i2c_id, cfg->i2c_address, &data, 1) != ECUAL_OK) {
        return APP_ERROR;
    }
    vTaskDelay(pdMS_TO_TICKS(1)); // Small delay for pulse width

    return APP_OK;
}

/**
 * @brief Sends a full byte (command or data) to the LCD in 4-bit mode.
 * @param unit_id The display unit ID.
 * @param byte The byte to send.
 * @param rs_mode RS_BIT (for data) or 0 (for command).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
static uint8_t lcd_send_byte(CHARACTER_DISPLAY_ID_t unit_id, uint8_t byte, uint8_t rs_mode) {
    uint8_t ret = APP_OK;
    // Send high nibble
    ret |= lcd_send_nibble(unit_id, byte & 0xF0, rs_mode);
    // Send low nibble
    ret |= lcd_send_nibble(unit_id, (byte & 0x0F) << 4, rs_mode);
    return ret;
}

/**
 * @brief Sends a command byte to the LCD.
 * @param unit_id The display unit ID.
 * @param cmd The command byte.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
static uint8_t lcd_send_cmd(CHARACTER_DISPLAY_ID_t unit_id, uint8_t cmd) {
    if (lcd_send_byte(unit_id, cmd, 0) != APP_OK) { // RS low for command
        ESP_LOGE(TAG, "Display %u: Failed to send command 0x%02X.", unit_id, cmd);
        return APP_ERROR;
    }
    return APP_OK;
}

/**
 * @brief Sends a data byte (character) to the LCD.
 * @param unit_id The display unit ID.
 * @param data The data byte.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
static uint8_t lcd_send_data(CHARACTER_DISPLAY_ID_t unit_id, uint8_t data) {
    if (lcd_send_byte(unit_id, data, RS_BIT) != APP_OK) { // RS high for data
        ESP_LOGE(TAG, "Display %u: Failed to send data 0x%02X.", unit_id, data);
        return APP_ERROR;
    }
    return APP_OK;
}

uint8_t CHARACTER_DISPLAY_Init(void) {
    uint8_t app_ret = APP_OK;

    for (uint32_t i = 0; i < CHARACTER_DISPLAY_NUM_CONFIGURATIONS; i++) {
        const CHARACTER_DISPLAY_Config_t *cfg = &character_display_configurations[i];
        CHARACTER_DISPLAY_State_t *state = &display_states[i];

        ESP_LOGI(TAG, "Initializing Display %u (I2C: %u, Addr: 0x%02X, %ux%u)",
                 i, cfg->i2c_id, cfg->i2c_address, cfg->num_columns, cfg->num_rows);

        // Store initial backlight state
        if (cfg->has_backlight) {
            state->backlight_state_mask = cfg->backlight_active_high ? BL_BIT : 0x00;
        } else {
            state->backlight_state_mask = 0x00; // No backlight control
        }

        // --- HD44780 Initialization Sequence (from datasheet, specific for 4-bit mode) ---
        // Reference: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf (page 46)

        // Step 1: Wait for more than 15ms after Vcc rises to 2.7V
        vTaskDelay(pdMS_TO_TICKS(50)); // Power-on delay (ensure enough time for display to stabilize)

        // Step 2: Function Set (first time, sends 0x30 three times)
        // This initializes the LCD into 8-bit mode, regardless of wiring.
        // We only send the upper nibble (0x30 >> 4 = 0x03) because it auto-detects 4-bit.
        if (lcd_send_nibble(i, 0x30, 0) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdMS_TO_TICKS(5)); // Wait for more than 4.1ms

        if (lcd_send_nibble(i, 0x30, 0) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdUS_TO_TICKS(150)); // Wait for more than 100us

        if (lcd_send_nibble(i, 0x30, 0) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdUS_TO_TICKS(150)); // Wait for more than 100us

        // Step 3: Function Set (set to 4-bit mode)
        if (lcd_send_nibble(i, 0x20, 0) != APP_OK) { app_ret = APP_ERROR; continue; } // Sets 4-bit mode
        vTaskDelay(pdUS_TO_TICKS(100)); // Short delay

        // --- Now in 4-bit mode, can send full commands ---

        // Function Set: 2 lines, 5x8 dots (most common)
        state->display_function_flags = LCD_4BITMODE | LCD_2LINE | (0x00); // 0x00 for 5x8 dots
        if (cfg->num_rows > 2) { // For 4-line displays, still configured as 2-line mode in hardware.
            state->display_function_flags = LCD_4BITMODE | LCD_2LINE | (0x00);
        }

        if (lcd_send_cmd(i, LCD_FUNCTIONSET | state->display_function_flags) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdUS_TO_TICKS(50));

        // Display Control: Display ON, Cursor OFF, Blink OFF
        state->display_control_flags = LCD_DISPLAYON;
        if (lcd_send_cmd(i, LCD_DISPLAYCONTROL | state->display_control_flags) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdUS_TO_TICKS(50));

        // Clear Display
        if (lcd_send_cmd(i, LCD_CLEARDISPLAY) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdMS_TO_TICKS(2)); // Clear display needs ~2ms

        // Entry Mode Set: Increment cursor, no display shift
        if (lcd_send_cmd(i, LCD_ENTRYMODESET | LCD_ENTRYSHIFTINCREMENT | LCD_ENTRYLEFT) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdUS_TO_TICKS(50));

        // Return Home
        if (lcd_send_cmd(i, LCD_RETURNHOME) != APP_OK) { app_ret = APP_ERROR; continue; }
        vTaskDelay(pdMS_TO_TICKS(2)); // Return home needs ~2ms

        // Turn on backlight if configured
        if (cfg->has_backlight) {
            if (CHARACTER_DISPLAY_BacklightOn(i) != APP_OK) { app_ret = APP_ERROR; }
        }

        state->current_col = 0;
        state->current_row = 0;

        ESP_LOGI(TAG, "Display %u initialization complete.", i);
    }
    return app_ret;
}

uint8_t CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ID_t unit_id) {
    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];

    if (lcd_send_cmd(unit_id, LCD_CLEARDISPLAY) != APP_OK) { return APP_ERROR; }
    vTaskDelay(pdMS_TO_TICKS(2)); // Clear display needs ~2ms

    state->current_col = 0;
    state->current_row = 0;
    ESP_LOGD(TAG, "Display %u cleared.", unit_id);
    return APP_OK;
}

uint8_t CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ID_t unit_id, uint8_t col, uint8_t row) {
    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];

    // Check bounds
    if (col >= cfg->num_columns || row >= cfg->num_rows) {
        ESP_LOGW(TAG, "Display %u: Cursor position (%u,%u) out of bounds (%ux%u).", unit_id, col, row, cfg->num_columns, cfg->num_rows);
        // Still try to set to a valid address if possible, or clamp.
        if (col >= cfg->num_columns) col = cfg->num_columns - 1;
        if (row >= cfg->num_rows) row = cfg->num_rows - 1;
    }

    uint8_t address = col;
    if (row < sizeof(row_offsets) / sizeof(row_offsets[0])) {
        address += row_offsets[row];
    } else {
        // Fallback for larger rows or if row_offsets doesn't cover
        // This is usually for 2 or 4 line displays. For more, specific handling needed.
        ESP_LOGW(TAG, "Display %u: Unsupported row %u for address calculation.", unit_id, row);
        return APP_ERROR;
    }

    if (lcd_send_cmd(unit_id, LCD_SETDDRAMADDR | address) != APP_OK) { return APP_ERROR; }
    state->current_col = col;
    state->current_row = row;
    ESP_LOGD(TAG, "Display %u cursor set to (%u,%u).", unit_id, col, row);
    return APP_OK;
}

uint8_t CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ID_t unit_id, const char *str) {
    if (str == NULL) {
        ESP_LOGE(TAG, "Display %u: NULL string passed to PrintString.", unit_id);
        return APP_ERROR;
    }

    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];
    uint8_t ret = APP_OK;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') { // Handle newline character
            state->current_row++;
            state->current_col = 0;
            if (state->current_row >= cfg->num_rows) {
                state->current_row = 0; // Wrap around to first row
            }
            if (CHARACTER_DISPLAY_SetCursor(unit_id, state->current_col, state->current_row) != APP_OK) {
                ret = APP_ERROR;
                break;
            }
        } else {
            // Check for line wrap
            if (state->current_col >= cfg->num_columns) {
                state->current_col = 0;
                state->current_row++;
                if (state->current_row >= cfg->num_rows) {
                    state->current_row = 0; // Wrap around to first row
                }
                if (CHARACTER_DISPLAY_SetCursor(unit_id, state->current_col, state->current_row) != APP_OK) {
                    ret = APP_ERROR;
                    break;
                }
            }

            if (lcd_send_data(unit_id, (uint8_t)c) != APP_OK) {
                ret = APP_ERROR;
                break;
            }
            state->current_col++;
        }
    }
    ESP_LOGD(TAG, "Display %u printed string: '%s'.", unit_id, str);
    return ret;
}

uint8_t CHARACTER_DISPLAY_PrintChar(CHARACTER_DISPLAY_ID_t unit_id, char c) {
    char s[2];
    s[0] = c;
    s[1] = '\0';
    return CHARACTER_DISPLAY_PrintString(unit_id, s);
}

uint8_t CHARACTER_DISPLAY_BacklightOn(CHARACTER_DISPLAY_ID_t unit_id) {
    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    if (!cfg->has_backlight) {
        ESP_LOGW(TAG, "Display %u: No controllable backlight configured.", unit_id);
        return APP_ERROR;
    }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];
    state->backlight_state_mask = cfg->backlight_active_high ? BL_BIT : 0x00;

    // Send a dummy command (e.g., set DDRAM address to current cursor) to update PCF8574 output.
    // The backlight bit is part of every transfer.
    // We send a byte with RS=0 and EN=0, so it's effectively a null operation to just update the backlight line.
    uint8_t data_byte = state->backlight_state_mask; // Only backlight bit, other bits 0.
    if (ECUAL_I2C_Master_Write(cfg->i2c_id, cfg->i2c_address, &data_byte, 1) != ECUAL_OK) {
        ESP_LOGE(TAG, "Display %u: Failed to turn backlight ON.", unit_id);
        return APP_ERROR;
    }
    ESP_LOGI(TAG, "Display %u backlight ON.", unit_id);
    return APP_OK;
}

uint8_t CHARACTER_DISPLAY_BacklightOff(CHARACTER_DISPLAY_ID_t unit_id) {
    const CHARACTER_DISPLAY_Config_t *cfg = display_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    if (!cfg->has_backlight) {
        ESP_LOGW(TAG, "Display %u: No controllable backlight configured.", unit_id);
        return APP_ERROR;
    }

    CHARACTER_DISPLAY_State_t *state = &display_states[unit_id];
    state->backlight_state_mask = cfg->backlight_active_high ? 0x00 : BL_BIT; // Inactive state

    // Send a dummy command to update PCF8574 output with new backlight state
    uint8_t data_byte = state->backlight_state_mask; // Only backlight bit, other bits 0.
    if (ECUAL_I2C_Master_Write(cfg->i2c_id, cfg->i2c_address, &data_byte, 1) != ECUAL_OK) {
        ESP_LOGE(TAG, "Display %u: Failed to turn backlight OFF.", unit_id);
        return APP_ERROR;
    }
    ESP_LOGI(TAG, "Display %u backlight OFF.", unit_id);
    return APP_OK;
}
