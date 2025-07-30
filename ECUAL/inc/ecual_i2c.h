// ecual/inc/ecual_i2c.h

#ifndef ECUAL_I2C_H
#define ECUAL_I2C_H

#include <stdint.h> // For uint8_t, uint16_t, uint32_t

// IMPORTANT: Directly include the MCAL driver header to align enum values.
#include "driver/i2c.h" // Provides I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_FREQ_HZ, etc.

/**
 * @brief Defines the I2C peripheral units.
 * Values are directly aligned with ESP-IDF's i2c_port_t.
 */
typedef enum {
    ECUAL_I2C_UNIT_0 = I2C_NUM_0, ///< I2C Controller 0
    ECUAL_I2C_UNIT_1 = I2C_NUM_1  ///< I2C Controller 1
} ECUAL_I2C_Unit_t;

/**
 * @brief Defines the I2C mode (Master or Slave).
 * Values are directly aligned with ESP-IDF's i2c_mode_t.
 * ECUAL focuses on Master mode primarily.
 */
typedef enum {
    ECUAL_I2C_MODE_MASTER = I2C_MODE_MASTER, ///< I2C Master mode
    // ECUAL_I2C_MODE_SLAVE = I2C_MODE_SLAVE, // Uncomment if slave mode support is needed later
} ECUAL_I2C_Mode_t;

/**
 * @brief Structure to hold the configuration for a single I2C unit.
 * Currently primarily for Master mode.
 */
typedef struct {
    ECUAL_I2C_Unit_t unit;          ///< I2C peripheral unit (0 or 1)
    ECUAL_I2C_Mode_t mode;          ///< I2C mode (ECUAL_I2C_MODE_MASTER)
    int              scl_pin;       ///< GPIO pin for SCL
    int              sda_pin;       ///< GPIO pin for SDA
    bool             scl_pullup_en; ///< Enable internal pull-up on SCL pin
    bool             sda_pullup_en; ///< Enable internal pull-up on SDA pin
    uint32_t         master_freq_hz;///< Master clock frequency in Hz (e.g., 100000, 400000)
    // Add slave address and buffer sizes if slave mode is implemented
} ECUAL_I2C_Config_t;

/**
 * @brief Initializes all I2C units based on the configurations defined in ecual_i2c_config.h.
 * @return ECUAL_OK if all units are initialized successfully, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_I2C_Init(void);

/**
 * @brief Performs an I2C master write transaction to a slave device.
 * @param unit The I2C unit to use.
 * @param dev_addr The 7-bit slave device address.
 * @param data Pointer to the data buffer to write.
 * @param len The number of bytes to write.
 * @param timeout_ms The timeout for the transaction in milliseconds.
 * @return ECUAL_OK if the write was successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_I2C_Master_Write(ECUAL_I2C_Unit_t unit, uint8_t dev_addr, const uint8_t *data, uint16_t len, uint32_t timeout_ms);

/**
 * @brief Performs an I2C master read transaction from a slave device.
 * @param unit The I2C unit to use.
 * @param dev_addr The 7-bit slave device address.
 * @param data Pointer to the buffer to store read data.
 * @param len The number of bytes to read.
 * @param timeout_ms The timeout for the transaction in milliseconds.
 * @return ECUAL_OK if the read was successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_I2C_Master_Read(ECUAL_I2C_Unit_t unit, uint8_t dev_addr, uint8_t *data, uint16_t len, uint32_t timeout_ms);

/**
 * @brief Performs an I2C master write-then-read transaction (e.g., for register read).
 * Writes data first (e.g., register address), then reads data without a stop condition in between.
 * @param unit The I2C unit to use.
 * @param dev_addr The 7-bit slave device address.
 * @param write_data Pointer to the data buffer to write.
 * @param write_len The number of bytes to write.
 * @param read_data Pointer to the buffer to store read data.
 * @param read_len The number of bytes to read.
 * @param timeout_ms The timeout for the entire transaction in milliseconds.
 * @return ECUAL_OK if the transaction was successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_I2C_Master_WriteRead(ECUAL_I2C_Unit_t unit, uint8_t dev_addr,
                                 const uint8_t *write_data, uint16_t write_len,
                                 uint8_t *read_data, uint16_t read_len,
                                 uint32_t timeout_ms);

#endif /* ECUAL_I2C_H */
