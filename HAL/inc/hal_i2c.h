/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_I2C.h
 * ============================================================================*/
/**
 * @file HAL_I2C.h
 * @brief Public API for I2C master operations and specific I/O expander control.
 * This header declares functions for initializing the I2C buses,
 * I2C read/write transactions, and control of the CH423S I/O expander.
 */
#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "common.h"     // For Status_t
#include "driver/i2c.h" // For i2c_port_t (used in public APIs for port selection)

/**
 * @brief Initializes all I2C master peripherals according to configurations
 * defined in `HAL_I2C_Cfg.c`. This is the main initialization function for the I2C HAL.
 * @return E_OK if initialization is successful, otherwise an error code.
 */
Status_t HAL_I2C_Init(void);

/**
 * @brief Performs an I2C master write operation.
 * @param i2c_port The I2C port number (e.g., HW_I2C_EXPANDER_PORT from hal_cfg.h).
 * @param i2c_addr The 7-bit I2C slave address.
 * @param write_buffer Pointer to the data to write.
 * @param write_len The number of bytes to write.
 * @param timeout_ms Timeout in milliseconds for the I2C transaction.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_I2C_MasterWrite(i2c_port_t i2c_port, uint8_t i2c_addr, const uint8_t *write_buffer, size_t write_len, uint32_t timeout_ms);

/**
 * @brief Performs an I2C master read operation.
 * @param i2c_port The I2C port number.
 * @param i2c_addr The 7-bit I2C slave address.
 * @param read_buffer Pointer to the buffer to store read data.
 * @param read_len The number of bytes to read.
 * @param timeout_ms Timeout in milliseconds for the I2C transaction.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_I2C_MasterRead(i2c_port_t i2c_port, uint8_t i2c_addr, uint8_t *read_buffer, size_t read_len, uint32_t timeout_ms);

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
                                 uint8_t *read_buffer, size_t read_len, uint32_t timeout_ms);

/**
 * @brief Sets the state of a specific GPIO pin on the CH423S I/O Expander.
 * This function handles the read-modify-write operation required for the CH423S,
 * using the predefined I2C port for the expander from hal_cfg.h.
 * @param gp_pin The GPIO pin number on the CH423S (0-15).
 * @param state The desired state (0 for low, 1 for high).
 * @return E_OK on success, or an error code if I2C communication fails.
 */
Status_t HAL_CH423S_SetOutput(uint8_t gp_pin, uint8_t state);

#endif /* HAL_I2C_H */
