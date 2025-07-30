// ecual/src/ecual_i2c.c

#include "ecual_i2c.h"        // Includes driver/i2c.h
#include "ecual_i2c_config.h" // For i2c_configurations array extern declaration
#include "ecual_common.h"     // For ECUAL_OK and ECUAL_ERROR

// Timeout conversion macro for ticks (assuming FreeRTOS tick is 1ms by default)
#define I2C_TIMEOUT_TICKS(ms) ((ms) / portTICK_PERIOD_MS)

uint8_t ECUAL_I2C_Init(void) {
    uint32_t i;
    esp_err_t mcal_ret;
    uint8_t ecual_ret = ECUAL_OK;

    for (i = 0; i < ECUAL_NUM_I2C_CONFIGURATIONS; i++) {
        const ECUAL_I2C_Config_t *cfg = &i2c_configurations[i];

        // 1. Configure I2C parameters
        i2c_config_t i2c_conf = {
            .mode             = (i2c_mode_t)cfg->mode, // Direct cast
            .sda_io_num       = cfg->sda_pin,
            .scl_io_num       = cfg->scl_pin,
            .sda_pullup_en    = cfg->sda_pullup_en,
            .scl_pullup_en    = cfg->scl_pullup_en,
            .master.clk_speed = cfg->master_freq_hz,
            // .slave.slave_addr and buffer sizes are not needed for master mode
        };

        mcal_ret = i2c_param_config((i2c_port_t)cfg->unit, &i2c_conf);
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            // Optionally: add logging here for which I2C unit failed
            continue;
        }

        // 2. Install I2C driver (no buffer needed for master mode, 0 for event_queue_size)
        mcal_ret = i2c_driver_install((i2c_port_t)cfg->unit, (i2c_mode_t)cfg->mode, 0, 0, 0);
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            continue;
        }
    }

    return ecual_ret;
}

uint8_t ECUAL_I2C_Master_Write(ECUAL_I2C_Unit_t unit, uint8_t dev_addr, const uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    esp_err_t mcal_ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Start condition
    i2c_master_start(cmd);
    // Write slave address + write bit
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true); // true for ACK check
    // Write data
    i2c_master_write(cmd, (uint8_t *)data, len, true); // true for ACK check for each byte
    // Stop condition
    i2c_master_stop(cmd);

    // Execute the command link
    mcal_ret = i2c_master_cmd_begin((i2c_port_t)unit, cmd, I2C_TIMEOUT_TICKS(timeout_ms));
    // Delete the command link to free memory
    i2c_cmd_link_delete(cmd);

    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_I2C_Master_Read(ECUAL_I2C_Unit_t unit, uint8_t dev_addr, uint8_t *data, uint16_t len, uint32_t timeout_ms) {
    esp_err_t mcal_ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Start condition
    i2c_master_start(cmd);
    // Write slave address + read bit
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true); // true for ACK check

    if (len > 1) {
        // Read multiple bytes, send ACK for all except the last one
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    // Read the last byte, send NACK
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    
    // Stop condition
    i2c_master_stop(cmd);

    // Execute the command link
    mcal_ret = i2c_master_cmd_begin((i2c_port_t)unit, cmd, I2C_TIMEOUT_TICKS(timeout_ms));
    // Delete the command link to free memory
    i2c_cmd_link_delete(cmd);

    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}


uint8_t ECUAL_I2C_Master_WriteRead(ECUAL_I2C_Unit_t unit, uint8_t dev_addr,
                                 const uint8_t *write_data, uint16_t write_len,
                                 uint8_t *read_data, uint16_t read_len,
                                 uint32_t timeout_ms) {
    esp_err_t mcal_ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // 1. Write phase
    // Start condition
    i2c_master_start(cmd);
    // Write slave address + write bit
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    // Write data (e.g., register address)
    i2c_master_write(cmd, (uint8_t *)write_data, write_len, true);

    // 2. Read phase (Repeated Start without Stop in between)
    // Repeated Start condition
    i2c_master_start(cmd);
    // Write slave address + read bit
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (read_len > 1) {
        // Read multiple bytes, send ACK for all except the last one
        i2c_master_read(cmd, read_data, read_len - 1, I2C_MASTER_ACK);
    }
    // Read the last byte, send NACK
    i2c_master_read_byte(cmd, read_data + read_len - 1, I2C_MASTER_NACK);

    // Stop condition for the entire transaction
    i2c_master_stop(cmd);

    // Execute the command link
    mcal_ret = i2c_master_cmd_begin((i2c_port_t)unit, cmd, I2C_TIMEOUT_TICKS(timeout_ms));
    // Delete the command link to free memory
    i2c_cmd_link_delete(cmd);

    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}
