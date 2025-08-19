/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_I2C.c
 * ============================================================================*/
/**
 * @file HAL_I2C.c
 * @brief Implements the public API functions for I2C master communication
 * and specific control for the CH423S I/O expander, including the module's
 * initialization function.
 * These functions wrap ESP-IDF I2C driver calls.
 */

#include "HAL_I2C.h"     // Header for HAL_I2C functions
#include "HAL_I2C_Cfg.h" // To access I2C configuration array
#include "hal_cfg.h"   // Global hardware definitions (CH423S address, I2C port)
#include "esp_log.h"     // ESP-IDF logging library
#include "driver/i2c.h"  // ESP-IDF I2C driver
#include "esp_err.h"     // For ESP_OK, ESP_FAIL, etc.

static const char *TAG = "HAL_I2C";

// --- Private Helper to manage CH423S 16-bit output state ---
// This static variable holds the current 16-bit output state of the CH423S.
// It's crucial for the read-modify-write operation if the CH423S acts like a PCF8575/MCP23017.
// IMPORTANT: You MUST consult the CH423S datasheet for its exact register map and behavior.
// This implementation assumes a simple 16-bit output register that is written to directly.
static uint16_t s_ch423s_output_state = 0x0000; // All outputs initially low on power-up/reset

/**
 * @brief Initializes the I2C master peripheral(s) according to configurations
 * defined in the internal `s_i2c_configurations` array from `HAL_I2C_Cfg.c`.
 * Iterates through the array and applies each I2C configuration.
 *
 * @return E_OK if initialization is successful, otherwise an error code.
 */
Status_t HAL_I2C_Init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying I2C configurations from HAL_I2C_Cfg.c...");

    for (size_t i = 0; i < s_num_i2c_configurations; i++)
    {
        const i2c_cfg_item_t *cfg_item = &s_i2c_configurations[i];

        ret = i2c_param_config(cfg_item->port, &cfg_item->config);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C param config for port %d failed: %s", cfg_item->port, esp_err_to_name(ret));
            return E_ERROR;
        }
        // No RX/TX buffer needed for master, last parameter is ISR service flags
        ret = i2c_driver_install(cfg_item->port, cfg_item->config.mode, 0, 0, 0);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C driver install for port %d failed: %s", cfg_item->port, esp_err_to_name(ret));
            return E_ERROR;
        }
        ESP_LOGD(TAG, "I2C master configured on port %d with SDA:%d, SCL:%d, Freq:%ldHz.",
                 cfg_item->port, cfg_item->config.sda_io_num, cfg_item->config.scl_io_num, cfg_item->config.master.clk_speed);
    }

    ESP_LOGI(TAG, "All I2C buses initialized successfully.");
    return E_OK;
}

/**
 * @brief Performs an I2C master write operation.
 * @param i2c_port The I2C port number (e.g., HW_I2C_EXPANDER_PORT).
 * @param i2c_addr The 7-bit I2C slave address.
 * @param write_buffer Pointer to the data to write.
 * @param write_len The number of bytes to write.
 * @param timeout_ms Timeout in milliseconds for the I2C transaction.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_I2C_MasterWrite(i2c_port_t i2c_port, uint8_t i2c_addr, const uint8_t *write_buffer, size_t write_len, uint32_t timeout_ms)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Failed to create I2C command link.");
        return E_ERROR;
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    if (write_len > 0)
    {
        i2c_master_write(cmd, (uint8_t *)write_buffer, write_len, true);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master Write failed for port %d, addr 0x%02X: %s", i2c_port, i2c_addr, esp_err_to_name(ret));
        return E_ERROR;
    }
    return E_OK;
}

/**
 * @brief Performs an I2C master read operation.
 * @param i2c_port The I2C port number.
 * @param i2c_addr The 7-bit I2C slave address.
 * @param read_buffer Pointer to the buffer to store read data.
 * @param read_len The number of bytes to read.
 * @param timeout_ms Timeout in milliseconds for the I2C transaction.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_I2C_MasterRead(i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t *read_buffer, size_t read_len, uint32_t timeout_ms)
{
    if (read_len == 0 || read_buffer == NULL)
    {
        ESP_LOGE(TAG, "I2C Master Read: Invalid read_len or NULL read_buffer.");
        return E_INVALID_PARAM;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Failed to create I2C command link.");
        return E_ERROR;
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    if (read_len > 1)
    {
        i2c_master_read(cmd, read_buffer, read_len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, read_buffer + read_len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master Read failed for port %d, addr 0x%02X: %s", i2c_port, i2c_addr, esp_err_to_name(ret));
        return E_ERROR;
    }
    return E_OK;
}

/**
 * @brief Performs an I2C master write-read operation (combined transaction).
 * Typically used for reading from specific registers.
 * @param i2c_port The I2C port number.
 * @param i2c_addr The 7-bit I2C slave address.
 * @param write_buffer Pointer to the data to write (e.g., register address).
 * @param write_len The number of bytes to write.
 * @param read_buffer Pointer to the buffer to store read data.
 * @param read_len The number of bytes to read.
 * @param timeout_ms Timeout in milliseconds for the I2C transaction.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_I2C_MasterWriteRead(i2c_port_t i2c_port, uint8_t i2c_addr, const uint8_t *write_buffer, size_t write_len,
                                 uint8_t *read_buffer, size_t read_len, uint32_t timeout_ms)
{
    if (read_len == 0 || read_buffer == NULL)
    {
        ESP_LOGE(TAG, "I2C Master Write-Read: Invalid read_len or NULL read_buffer.");
        return E_INVALID_PARAM;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL)
    {
        ESP_LOGE(TAG, "Failed to create I2C command link.");
        return E_ERROR;
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    if (write_len > 0)
    {
        i2c_master_write(cmd, (uint8_t *)write_buffer, write_len, true);
    }
    i2c_master_start(cmd); // Repeated start
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    if (read_len > 1)
    {
        i2c_master_read(cmd, read_buffer, read_len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, read_buffer + read_len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master Write-Read failed for port %d, addr 0x%02X: %s", i2c_port, i2c_addr, esp_err_to_name(ret));
        return E_ERROR;
    }
    return E_OK;
}

/**
 * @brief Sets the state of a specific GPIO pin on the CH423S I/O Expander.
 *
 * This function updates the internal 16-bit state of the CH423S outputs and
 * then writes this full 16-bit value to the expander via I2C. This read-modify-write
 * pattern (conceptually, as we track the state internally) is common for expanders.
 *
 * IMPORTANT: This implementation assumes the CH423S behaves like a standard
 * 16-bit I/O expander where you write two bytes directly to control all 16 pins
 * (e.g., GP0-7 in the first byte, GP8-15 in the second byte). You **MUST**
 * consult the CH423S datasheet for the exact register addresses and bit manipulation protocol.
 * The `HW_CH423S_I2C_ADDR` is used from `hal_cfg.h`.
 *
 * @param gp_pin The GPIO pin number on the CH423S (0-15).
 * @param state The desired state (0 for low, 1 for high).
 * @return E_OK on success, or an error code if I2C communication fails.
 */
Status_t HAL_CH423S_SetOutput(uint8_t gp_pin, uint8_t state)
{
    if (gp_pin > 15)
    {
        ESP_LOGE(TAG, "Invalid CH423S GP pin: %d. Must be 0-15.", gp_pin);
        return E_INVALID_PARAM;
    }

    // Update the internal 16-bit state representation
    if (state)
    {
        s_ch423s_output_state |= (1 << gp_pin); // Set the bit
    }
    else
    {
        s_ch423s_output_state &= ~(1 << gp_pin); // Clear the bit
    }

    // Prepare the two bytes to send to the CH423S (assuming little-endian byte order)
    uint8_t write_data[2];
    write_data[0] = (uint8_t)(s_ch423s_output_state & 0xFF);        // Low byte (GP0-7)
    write_data[1] = (uint8_t)((s_ch423s_output_state >> 8) & 0xFF); // High byte (GP8-15)

    // Send the 16-bit data to the CH423S using its dedicated I2C port
    Status_t status = HAL_I2C_MasterWrite(HW_I2C_EXPANDER_PORT, HW_CH423S_I2C_ADDR, write_data, sizeof(write_data), 100);

    if (status != E_OK)
    {
        ESP_LOGE(TAG, "Failed to set CH423S GP%d to %d (I2C error).", gp_pin, state);
        return status;
    }

    ESP_LOGD(TAG, "CH423S GP%d set to %d. Current state: 0x%04X", gp_pin, state, s_ch423s_output_state);
    return E_OK;
}
