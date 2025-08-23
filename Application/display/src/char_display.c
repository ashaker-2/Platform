/* ============================================================================
 * SOURCE FILE: char_display.c
 * ============================================================================*/
/**
 * @file char_display.c
 * @brief Implements the character display driver for 4-bit mode LCDs.
 *
 * This file provides the low-level functions to control a character LCD
 * display using the ESP-IDF GPIO driver. It handles the 4-bit communication
 * protocol, including initialization, sending commands, and writing data.
 */

#include "char_display.h"      // Public API for character display
#include "char_display_cfg.h"  // Configuration for pin assignments
#include "driver/gpio.h"       // ESP-IDF GPIO driver
#include "esp_log.h"           // ESP-IDF logging
#include "esp_rom_sys.h"       // For esp_rom_delay_us
#include <string.h>            // For strlen

// --- Defines for LCD Commands and flags ---
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// Entry Mode Set flags
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Display Control flags
#define LCD_DISPLAYON           0x04
#define LCD_CURSORON            0x02
#define LCD_BLINKON             0x01

// Cursor Shift flags
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// Function Set flags
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

// Backlight control - not implemented in this version, assuming always on or external control

// --- Module Variables ---
static const char_display_cfg_t *s_display_cfg = NULL; // Pointer to the active configuration
static uint8_t s_display_control_flags = 0; // Stores current state of display, cursor, blink
static const char *TAG = "char_display"; // Logging tag

// --- Helper Functions (Private) ---

/**
 * @brief Provides a microsecond delay.
 * @param us The number of microseconds to delay.
 */
static void delay_us(uint32_t us) {
    esp_rom_delay_us(us);
}

/**
 * @brief Sets the Enable pin high, then low, to clock data into the LCD.
 * @param delay_ns_after_e_high Delay in nanoseconds after setting E high.
 * @param delay_ns_after_e_low Delay in nanoseconds after setting E low.
 */
static void pulse_enable(void) {
    gpio_set_level(s_display_cfg->e_pin, 1);
    delay_us(1); // Short pulse width, at least 450ns
    gpio_set_level(s_display_cfg->e_pin, 0);
    delay_us(50); // Data setup time, min 37us
}

/**
 * @brief Sends a 4-bit nibble to the LCD's data pins.
 * @param nibble The 4-bit value to send (only bits 0-3 are used).
 */
static void send_nibble(uint8_t nibble) {
    gpio_set_level(s_display_cfg->db4_pin, (nibble >> 0) & 0x01);
    gpio_set_level(s_display_cfg->db5_pin, (nibble >> 1) & 0x01);
    gpio_set_level(s_display_cfg->db6_pin, (nibble >> 2) & 0x01);
    gpio_set_level(s_display_cfg->db7_pin, (nibble >> 3) & 0x01);
}

/**
 * @brief Sends a full 8-bit command to the LCD.
 * This function handles the 4-bit interface protocol by sending the higher nibble first,
 * then the lower nibble. RS pin is kept low for commands.
 * @param cmd The 8-bit command to send.
 */
static void send_command(uint8_t cmd) {
    gpio_set_level(s_display_cfg->rs_pin, 0); // RS low for command
    send_nibble(cmd >> 4); // Send high nibble
    pulse_enable();
    send_nibble(cmd & 0x0F); // Send low nibble
    pulse_enable();
    delay_us(100); // Most commands take ~37us, clear and home take 1.52ms
}

/**
 * @brief Sends a full 8-bit data byte to the LCD.
 * This function handles the 4-bit interface protocol by sending the higher nibble first,
 * then the lower nibble. RS pin is kept high for data.
 * @param data The 8-bit data byte (character ASCII value) to send.
 */
static void send_data(uint8_t data) {
    gpio_set_level(s_display_cfg->rs_pin, 1); // RS high for data
    send_nibble(data >> 4); // Send high nibble
    pulse_enable();
    send_nibble(data & 0x0F); // Send low nibble
    pulse_enable();
    delay_us(50); // Data write typically 43us
}

/**
 * @brief Initializes the GPIO pins for the character display.
 * @return ESP_OK on success, or an error code.
 */
static esp_err_t gpio_init(void) {
    esp_err_t ret = ESP_OK;

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    io_conf.pin_bit_mask = (1ULL << s_display_cfg->rs_pin) |
                           (1ULL << s_display_cfg->e_pin) |
                           (1ULL << s_display_cfg->db4_pin) |
                           (1ULL << s_display_cfg->db5_pin) |
                           (1ULL << s_display_cfg->db6_pin) |
                           (1ULL << s_display_cfg->db7_pin);

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO for display: %s", esp_err_to_name(ret));
        return ret;
    }

    // Ensure all pins are low initially
    gpio_set_level(s_display_cfg->rs_pin, 0);
    gpio_set_level(s_display_cfg->e_pin, 0);
    gpio_set_level(s_display_cfg->db4_pin, 0);
    gpio_set_level(s_display_cfg->db5_pin, 0);
    gpio_set_level(s_display_cfg->db6_pin, 0);
    gpio_set_level(s_display_cfg->db7_pin, 0);

    ESP_LOGI(TAG, "GPIOs configured for character display.");
    return ESP_OK;
}

// --- Public Functions ---

esp_err_t HAL_CharDisplay_Init(void) {
    s_display_cfg = &s_char_display_config; // Get the configuration

    esp_err_t ret = gpio_init();
    if (ret != ESP_OK) {
        return ret;
    }

    // LCD initialization sequence (4-bit mode)
    // Ref: HD44780U datasheet, 4-bit initialization sequence

    delay_us(50000); // Wait >40ms after Vdd rises to 2.7V

    // 1. Function set (8-bit interface) - 0x30
    // Try sending this command 3 times to ensure LCD is in a known state
    gpio_set_level(s_display_cfg->rs_pin, 0);
    send_nibble(0x03);
    pulse_enable();
    delay_us(4500); // Wait >4.1ms

    // 2. Function set (8-bit interface) - 0x30
    send_nibble(0x03);
    pulse_enable();
    delay_us(150); // Wait >100us

    // 3. Function set (8-bit interface) - 0x30
    send_nibble(0x03);
    pulse_enable();
    // No delay needed after third 0x30, as next command is different

    // 4. Function set (4-bit interface) - 0x20
    send_nibble(0x02); // Set to 4-bit mode
    pulse_enable();
    delay_us(50); // Command execution time

    // Now LCD is in 4-bit mode, send full 8-bit commands using send_command()

    // 5. Function Set: 2 lines, 5x8 dots (0x28)
    send_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    s_display_control_flags = LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON; // Default to ON to be consistent with future calls
    // Initialize with display ON, cursor OFF, blink OFF
    HAL_CharDisplay_EnableDisplay(true); // Sets LCD_DISPLAYON
    HAL_CharDisplay_EnableCursor(false); // Clears LCD_CURSORON
    HAL_CharDisplay_EnableBlink(false);  // Clears LCD_BLINKON

    // 6. Clear Display (0x01)
    HAL_CharDisplay_ClearDisplay();
    delay_us(2000); // This command takes longer (>1.52ms)

    // 7. Entry Mode Set: Increment cursor, no display shift (0x06)
    send_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);

    ESP_LOGI(TAG, "Character display initialized successfully.");
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_WriteChar(char c) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    send_data(c);
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_WriteString(const char *str) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    if (str == NULL) {
        ESP_LOGW(TAG, "Attempted to write NULL string.");
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < strlen(str); i++) {
        send_data(str[i]);
    }
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_SetCursor(uint8_t col, uint8_t row) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }

    uint8_t address;
    switch (row) {
        case 0:
            address = 0x00 + col;
            break;
        case 1:
            address = 0x40 + col;
            break;
        // Add more cases for 3rd/4th lines if using a 4-line display, e.g.:
        // case 2: address = 0x14 + col; break;
        // case 3: address = 0x54 + col; break;
        default:
            ESP_LOGW(TAG, "Invalid row: %d. Setting to row 0.", row);
            address = 0x00 + col;
            break;
    }
    send_command(LCD_SETDDRAMADDR | address);
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_ClearDisplay(void) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    send_command(LCD_CLEARDISPLAY);
    delay_us(2000); // Clear display command takes longer
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_Home(void) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    send_command(LCD_RETURNHOME);
    delay_us(2000); // Return home command takes longer
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_EnableDisplay(bool enable) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    if (enable) {
        s_display_control_flags |= LCD_DISPLAYON;
    } else {
        s_display_control_flags &= ~LCD_DISPLAYON;
    }
    send_command(LCD_DISPLAYCONTROL | s_display_control_flags);
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_EnableBlink(bool enable) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    if (enable) {
        s_display_control_flags |= LCD_BLINKON;
    } else {
        s_display_control_flags &= ~LCD_BLINKON;
    }
    send_command(LCD_DISPLAYCONTROL | s_display_control_flags);
    return ESP_OK;
}

esp_err_t HAL_CharDisplay_EnableCursor(bool enable) {
    if (s_display_cfg == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return ESP_FAIL;
    }
    if (enable) {
        s_display_control_flags |= LCD_CURSORON;
    } else {
        s_display_control_flags &= ~LCD_CURSORON;
    }
    send_command(LCD_DISPLAYCONTROL | s_display_control_flags);
    return ESP_OK;
}
