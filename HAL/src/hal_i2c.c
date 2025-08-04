/**
 * @file hal_i2c.c
 * @brief Hardware Abstraction Layer for I2C - Implementation.
 *
 * This module provides the concrete implementation for the I2C HAL interface
 * defined in `hal_i2c.h`. It utilizes the ESP-IDF I2C driver to
 * configure and manage I2C master functionalities on the ESP32.
 */

#include "hal_i2c.h"
#include "hal_i2c_cfg.h" // Include configuration header

#include "driver/i2c.h"        // ESP-IDF I2C driver
#include "esp_log.h"           // ESP-IDF logging
#include "freertos/FreeRTOS.h" // For FreeRTOS types (e.g., tick conversion)

// Define a tag for ESP-IDF logging
static const char *TAG = "HAL_I2C";

/**
 * @brief Structure to hold runtime data for each I2C port.
 */
typedef struct
{
    bool initialized;        /**< Flag indicating if the port is initialized. */
    i2c_port_t esp_i2c_port; /**< ESP-IDF I2C port number. */
} HAL_I2C_PortData_t;

// Array to hold runtime data for each I2C port
static HAL_I2C_PortData_t g_i2c_port_data[HAL_I2C_PORT_MAX];

/**
 * @brief Internal helper to map HAL_I2C_Port_t to ESP-IDF i2c_port_t.
 * @param port The HAL I2C port.
 * @param esp_port Pointer to store the ESP-IDF I2C port number.
 * @return true if mapping is successful, false otherwise.
 */
static bool hal_i2c_map_port_to_esp_port(HAL_I2C_Port_t port, i2c_port_t *esp_port)
{
    switch (port)
    {
    case HAL_I2C_PORT_0:
        *esp_port = I2C_NUM_0;
        break;
    case HAL_I2C_PORT_1:
        *esp_port = I2C_NUM_1;
        break;
    default:
        ESP_LOGE(TAG, "Invalid HAL I2C port: %d", port);
        return false;
    }
    return true;
}

/**
 * @brief Internal helper to convert ESP-IDF error codes to HAL_I2C_Status_t.
 * @param esp_err The ESP-IDF error code.
 * @return The corresponding HAL_I2C_Status_t.
 */
static HAL_I2C_Status_t hal_i2c_esp_err_to_status(esp_err_t esp_err)
{
    switch (esp_err)
    {
    case ESP_OK:
        return HAL_I2C_STATUS_OK;
    case ESP_ERR_INVALID_ARG:
        return HAL_I2C_STATUS_INVALID_PARAM;
    case ESP_FAIL: // Generic failure, often due to NACK or timeout
        return HAL_I2C_STATUS_ERROR;
    case ESP_ERR_TIMEOUT:
        return HAL_I2C_STATUS_TIMEOUT;
    case ESP_ERR_INVALID_STATE: // I2C not initialized or in wrong state
        return HAL_I2C_STATUS_NOT_INITIALIZED;
    // Specific I2C errors (from i2c_cmd_link_create_static etc.)
    case I2C_MASTER_ERR_NACK_ADDR:
        return HAL_I2C_STATUS_NACK_ADDR;
    case I2C_MASTER_ERR_NACK_DATA:
        return HAL_I2C_STATUS_NACK_DATA;
    case I2C_MASTER_ERR_TIMEOUT:
        return HAL_I2C_STATUS_TIMEOUT;
    default:
        return HAL_I2C_STATUS_ERROR;
    }
}

HAL_I2C_Status_t HAL_I2C_Master_Init(HAL_I2C_Port_t port, const HAL_I2C_MasterConfig_t *config)
{
    if (port >= HAL_I2C_PORT_MAX)
    {
        ESP_LOGE(TAG, "Invalid I2C port: %d", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }
    if (config == NULL)
    {
        ESP_LOGE(TAG, "I2C master config is NULL for port %d.", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }
    if (g_i2c_port_data[port].initialized)
    {
        ESP_LOGW(TAG, "I2C port %d already initialized.", port);
        return HAL_I2C_STATUS_ALREADY_INITIALIZED;
    }

    i2c_port_t esp_i2c_port;
    if (!hal_i2c_map_port_to_esp_port(port, &esp_i2c_port))
    {
        return HAL_I2C_STATUS_INVALID_PARAM;
    }

    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config->sda_pin,
        .scl_io_num = config->scl_pin,
        .sda_pullup_en = g_hal_i2c_port_configs[port].pullup_en, // Use config from hal_i2c_cfg.c
        .scl_pullup_en = g_hal_i2c_port_configs[port].pullup_en, // Use config from hal_i2c_cfg.c
        .master.clk_speed = config->clk_speed_hz,
        .clk_flags = 0, // No clock flags
    };

    esp_err_t ret;

    ret = i2c_param_config(esp_i2c_port, &i2c_cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure I2C port %d parameters: %s", port, esp_err_to_name(ret));
        return hal_i2c_esp_err_to_status(ret);
    }

    ret = i2c_driver_install(esp_i2c_port, i2c_cfg.mode, 0, 0, 0); // No RX/TX buffer for master
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install I2C driver for port %d: %s", port, esp_err_to_name(ret));
        return hal_i2c_esp_err_to_status(ret);
    }

    // Set timeout for the I2C bus (in APB clock cycles)
    // APB_CLK is typically 80MHz.
    // Timeout in ticks = timeout_ms * (APB_CLK_FREQ_HZ / 1000)
    // ESP-IDF i2c_set_timeout takes ticks directly.
    uint32_t timeout_ticks = (config->timeout_ms * (APB_CLK_FREQ / 1000));
    ret = i2c_set_timeout(esp_i2c_port, timeout_ticks);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set I2C timeout for port %d: %s", port, esp_err_to_name(ret));
        i2c_driver_delete(esp_i2c_port);
        return hal_i2c_esp_err_to_status(ret);
    }

    g_i2c_port_data[port].initialized = true;
    g_i2c_port_data[port].esp_i2c_port = esp_i2c_port;

    LOGI(TAG, "I2C port %d initialized (SDA:%d, SCL:%d, Speed:%luHz, Timeout:%lu ms).",
         port, config->sda_pin, config->scl_pin, config->clk_speed_hz, config->timeout_ms);
    return HAL_I2C_STATUS_OK;
}

HAL_I2C_Status_t HAL_I2C_DeInit(HAL_I2C_Port_t port)
{
    if (port >= HAL_I2C_PORT_MAX)
    {
        ESP_LOGE(TAG, "Invalid I2C port: %d", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }
    if (!g_i2c_port_data[port].initialized)
    {
        ESP_LOGW(TAG, "I2C port %d not initialized.", port);
        return HAL_I2C_STATUS_OK; // Already de-initialized or never initialized
    }

    i2c_port_t esp_i2c_port = g_i2c_port_data[port].esp_i2c_port;
    esp_err_t ret = i2c_driver_delete(esp_i2c_port);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete I2C driver for port %d: %s", port, esp_err_to_name(ret));
        return hal_i2c_esp_err_to_status(ret);
    }

    g_i2c_port_data[port].initialized = false;
    LOGI(TAG, "I2C port %d de-initialized successfully.", port);
    return HAL_I2C_STATUS_OK;
}

HAL_I2C_Status_t HAL_I2C_Master_Write(HAL_I2C_Port_t port, uint8_t slave_addr, const uint8_t *data_write, size_t size, uint32_t timeout_ms)
{
    if (port >= HAL_I2C_PORT_MAX || !g_i2c_port_data[port].initialized)
    {
        ESP_LOGE(TAG, "I2C port %d not initialized or invalid.", port);
        return HAL_I2C_STATUS_NOT_INITIALIZED;
    }
    if (data_write == NULL || size == 0)
    {
        ESP_LOGE(TAG, "Data buffer is NULL or size is 0 for I2C write on port %d.", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }

    i2c_port_t esp_i2c_port = g_i2c_port_data[port].esp_i2c_port;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = ESP_OK;

    // Start condition
    ret = i2c_master_start(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Slave address + Write bit
    ret = i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Write data
    ret = i2c_master_write(cmd, (uint8_t *)data_write, size, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Stop condition
    ret = i2c_master_stop(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Execute commands
    ret = i2c_master_cmd_begin(esp_i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));

end:
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master Write failed on port %d (0x%02X): %s", port, slave_addr, esp_err_to_name(ret));
    }
    return hal_i2c_esp_err_to_status(ret);
}

HAL_I2C_Status_t HAL_I2C_Master_Read(HAL_I2C_Port_t port, uint8_t slave_addr, uint8_t *data_read, size_t size, uint32_t timeout_ms)
{
    if (port >= HAL_I2C_PORT_MAX || !g_i2c_port_data[port].initialized)
    {
        ESP_LOGE(TAG, "I2C port %d not initialized or invalid.", port);
        return HAL_I2C_STATUS_NOT_INITIALIZED;
    }
    if (data_read == NULL || size == 0)
    {
        ESP_LOGE(TAG, "Data buffer is NULL or size is 0 for I2C read on port %d.", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }

    i2c_port_t esp_i2c_port = g_i2c_port_data[port].esp_i2c_port;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = ESP_OK;

    // Start condition
    ret = i2c_master_start(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Slave address + Read bit
    ret = i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    if (size > 1)
    {
        // Read (size - 1) bytes with ACK
        ret = i2c_master_read(cmd, data_read, size - 1, I2C_MASTER_ACK);
        if (ret != ESP_OK)
        {
            goto end;
        }
    }
    // Read the last byte with NACK
    ret = i2c_master_read_byte(cmd, data_read + size - 1, I2C_MASTER_NACK);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Stop condition
    ret = i2c_master_stop(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Execute commands
    ret = i2c_master_cmd_begin(esp_i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));

end:
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master Read failed on port %d (0x%02X): %s", port, slave_addr, esp_err_to_name(ret));
    }
    return hal_i2c_esp_err_to_status(ret);
}

HAL_I2C_Status_t HAL_I2C_Master_WriteRead(HAL_I2C_Port_t port, uint8_t slave_addr,
                                          const uint8_t *write_data, size_t write_size,
                                          uint8_t *read_data, size_t read_size,
                                          uint32_t timeout_ms)
{
    if (port >= HAL_I2C_PORT_MAX || !g_i2c_port_data[port].initialized)
    {
        ESP_LOGE(TAG, "I2C port %d not initialized or invalid.", port);
        return HAL_I2C_STATUS_NOT_INITIALIZED;
    }
    if (write_data == NULL || write_size == 0 || read_data == NULL || read_size == 0)
    {
        ESP_LOGE(TAG, "Invalid data buffers or sizes for I2C write-read on port %d.", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }

    i2c_port_t esp_i2c_port = g_i2c_port_data[port].esp_i2c_port;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = ESP_OK;

    // Start condition
    ret = i2c_master_start(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Slave address + Write bit
    ret = i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_WRITE, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Write data
    ret = i2c_master_write(cmd, (uint8_t *)write_data, write_size, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Repeated Start condition
    ret = i2c_master_start(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Slave address + Read bit
    ret = i2c_master_write_byte(cmd, (slave_addr << 1) | I2C_MASTER_READ, true);
    if (ret != ESP_OK)
    {
        goto end;
    }

    if (read_size > 1)
    {
        // Read (read_size - 1) bytes with ACK
        ret = i2c_master_read(cmd, read_data, read_size - 1, I2C_MASTER_ACK);
        if (ret != ESP_OK)
        {
            goto end;
        }
    }
    // Read the last byte with NACK
    ret = i2c_master_read_byte(cmd, read_data + read_size - 1, I2C_MASTER_NACK);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Stop condition
    ret = i2c_master_stop(cmd);
    if (ret != ESP_OK)
    {
        goto end;
    }

    // Execute commands
    ret = i2c_master_cmd_begin(esp_i2c_port, cmd, pdMS_TO_TICKS(timeout_ms));

end:
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C Master WriteRead failed on port %d (0x%02X): %s", port, slave_addr, esp_err_to_name(ret));
    }
    return hal_i2c_esp_err_to_status(ret);
}

HAL_I2C_Status_t HAL_I2C_Master_ScanBus(HAL_I2C_Port_t port, uint8_t *found_addresses, uint8_t max_addresses, uint8_t *actual_found, uint32_t timeout_per_addr_ms)
{
    if (port >= HAL_I2C_PORT_MAX || !g_i2c_port_data[port].initialized)
    {
        ESP_LOGE(TAG, "I2C port %d not initialized or invalid.", port);
        return HAL_I2C_STATUS_NOT_INITIALIZED;
    }
    if (found_addresses == NULL || actual_found == NULL || max_addresses == 0)
    {
        ESP_LOGE(TAG, "Invalid parameters for I2C bus scan on port %d.", port);
        return HAL_I2C_STATUS_INVALID_PARAM;
    }

    i2c_port_t esp_i2c_port = g_i2c_port_data[port].esp_i2c_port;
    *actual_found = 0;
    LOGI(TAG, "Scanning I2C bus %d...", port);

    for (int addr = 1; addr < 127; addr++) // 7-bit addresses, 0x00 and 0x7F are reserved
    {
        if (*actual_found >= max_addresses)
        {
            ESP_LOGW(TAG, "Max addresses (%d) reached during scan. Stopping.", max_addresses);
            break;
        }

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        esp_err_t ret = ESP_OK;

        ret = i2c_master_start(cmd);
        if (ret != ESP_OK)
        {
            i2c_cmd_link_delete(cmd);
            continue;
        }

        ret = i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        if (ret != ESP_OK)
        {
            i2c_cmd_link_delete(cmd);
            continue;
        } // NACK, or other error

        ret = i2c_master_stop(cmd);
        if (ret != ESP_OK)
        {
            i2c_cmd_link_delete(cmd);
            continue;
        }

        ret = i2c_master_cmd_begin(esp_i2c_port, cmd, pdMS_TO_TICKS(timeout_per_addr_ms));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK)
        {
            found_addresses[*actual_found] = (uint8_t)addr;
            (*actual_found)++;
            LOGI(TAG, "Found device at 0x%02X", addr);
        }
        else if (ret == ESP_ERR_TIMEOUT)
        {
            // Timeout, device not responding or bus issue, ignore and continue
            // ESP_LOGD(TAG, "No response from 0x%02X (timeout)", addr);
        }
        else if (ret == I2C_MASTER_ERR_NACK_ADDR)
        {
            // No device at this address, expected for most addresses
            // ESP_LOGD(TAG, "No ACK from 0x%02X", addr);
        }
        else
        {
            ESP_LOGW(TAG, "I2C scan error at 0x%02X: %s", addr, esp_err_to_name(ret));
        }
    }

    LOGI(TAG, "I2C bus scan completed. Found %d devices.", *actual_found);
    return HAL_I2C_STATUS_OK;
}

     