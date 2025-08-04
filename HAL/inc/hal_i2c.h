/**
 * @file hal_i2c.h
 * @brief Hardware Abstraction Layer for I2C - Interface Header.
 *
 * This header defines the public API for the I2C HAL, providing an
 * abstract interface for I2C functionalities such as initialization,
 * master read/write operations, and bus scanning.
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for I2C HAL status codes.
 */
typedef enum
{
    HAL_I2C_STATUS_OK = 0,             /**< Operation successful. */
    HAL_I2C_STATUS_ERROR,              /**< Generic error. */
    HAL_I2C_STATUS_INVALID_PARAM,      /**< Invalid parameter provided. */
    HAL_I2C_STATUS_NOT_INITIALIZED,    /**< I2C HAL not initialized for the port. */
    HAL_I2C_STATUS_ALREADY_INITIALIZED,/**< I2C HAL already initialized for the port. */
    HAL_I2C_STATUS_BUSY,               /**< I2C bus is busy. */
    HAL_I2C_STATUS_TIMEOUT,            /**< Operation timed out. */
    HAL_I2C_STATUS_NACK_ADDR,          /**< No acknowledge from slave address. */
    HAL_I2C_STATUS_NACK_DATA,          /**< No acknowledge from slave data. */
    HAL_I2C_STATUS_MAX                 /**< Maximum number of status codes. */
} HAL_I2C_Status_t;

/**
 * @brief Enumeration for available I2C ports/channels.
 *
 * These map to underlying hardware I2C controllers (e.g., I2C_NUM_0, I2C_NUM_1 on ESP32).
 */
typedef enum
{
    HAL_I2C_PORT_0 = 0,         /**< I2C Port 0. */
    HAL_I2C_PORT_1,             /**< I2C Port 1. */
    HAL_I2C_PORT_MAX            /**< Maximum number of I2C ports. */
} HAL_I2C_Port_t;

/**
 * @brief Enumeration for I2C master clock speeds.
 */
typedef enum
{
    HAL_I2C_CLK_SPEED_100KHZ = 100000, /**< Standard Mode (100 kHz). */
    HAL_I2C_CLK_SPEED_400KHZ = 400000, /**< Fast Mode (400 kHz). */
    HAL_I2C_CLK_SPEED_1MHZ   = 1000000 /**< Fast Mode Plus (1 MHz). */
} HAL_I2C_ClockSpeed_t;

/**
 * @brief Structure for I2C master configuration.
 */
typedef struct
{
    int scl_pin;                /**< GPIO pin number for SCL. */
    int sda_pin;                /**< GPIO pin number for SDA. */
    HAL_I2C_ClockSpeed_t clk_speed_hz; /**< Master clock speed in Hz. */
    uint32_t timeout_ms;        /**< Bus timeout in milliseconds. */
} HAL_I2C_MasterConfig_t;

/**
 * @brief Initializes a specific I2C port in master mode.
 *
 * This function configures the specified I2C port as a master, setting up
 * the GPIO pins, clock speed, and bus timeout.
 *
 * @param port The I2C port to initialize.
 * @param config Pointer to the master configuration structure.
 * @return HAL_I2C_STATUS_OK if successful, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_Master_Init(HAL_I2C_Port_t port, const HAL_I2C_MasterConfig_t *config);

/**
 * @brief De-initializes a specific I2C port.
 *
 * This function frees resources associated with the I2C port and disables it.
 *
 * @param port The I2C port to de-initialize.
 * @return HAL_I2C_STATUS_OK if successful, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_DeInit(HAL_I2C_Port_t port);

/**
 * @brief Performs an I2C master write operation to a slave device.
 *
 * This function writes a buffer of data to a specified slave address.
 *
 * @param port The I2C port to use.
 * @param slave_addr The 7-bit slave address.
 * @param data_write Pointer to the data buffer to write.
 * @param size The number of bytes to write.
 * @param timeout_ms Timeout for the operation in milliseconds. Use 0 for no timeout.
 * @return HAL_I2C_STATUS_OK if successful, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_Master_Write(HAL_I2C_Port_t port, uint8_t slave_addr, const uint8_t *data_write, size_t size, uint32_t timeout_ms);

/**
 * @brief Performs an I2C master read operation from a slave device.
 *
 * This function reads a specified number of bytes from a slave device.
 *
 * @param port The I2C port to use.
 * @param slave_addr The 7-bit slave address.
 * @param data_read Pointer to the buffer to store the read data.
 * @param size The number of bytes to read.
 * @param timeout_ms Timeout for the operation in milliseconds. Use 0 for no timeout.
 * @return HAL_I2C_STATUS_OK if successful, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_Master_Read(HAL_I2C_Port_t port, uint8_t slave_addr, uint8_t *data_read, size_t size, uint32_t timeout_ms);

/**
 * @brief Performs an I2C master write-then-read operation.
 *
 * This function first writes a command/register address, then reads data
 * from the same slave device without releasing the bus.
 *
 * @param port The I2C port to use.
 * @param slave_addr The 7-bit slave address.
 * @param write_data Pointer to the data buffer to write (e.g., register address).
 * @param write_size The number of bytes to write.
 * @param read_data Pointer to the buffer to store the read data.
 * @param read_size The number of bytes to read.
 * @param timeout_ms Timeout for the entire operation in milliseconds.
 * @return HAL_I2C_STATUS_OK if successful, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_Master_WriteRead(HAL_I2C_Port_t port, uint8_t slave_addr,
                                          const uint8_t *write_data, size_t write_size,
                                          uint8_t *read_data, size_t read_size,
                                          uint32_t timeout_ms);

/**
 * @brief Scans the I2C bus for connected slave devices.
 *
 * This function attempts to communicate with all possible 7-bit I2C addresses
 * and reports which ones acknowledge.
 *
 * @param port The I2C port to scan.
 * @param found_addresses Array to store the 7-bit addresses of found devices.
 * @param max_addresses The maximum number of addresses that can be stored in the array.
 * @param actual_found Pointer to a variable that will store the actual number of devices found.
 * @param timeout_per_addr_ms Timeout for each address probe in milliseconds.
 * @return HAL_I2C_STATUS_OK if scan completes, an error code otherwise.
 */
HAL_I2C_Status_t HAL_I2C_Master_ScanBus(HAL_I2C_Port_t port, uint8_t *found_addresses, uint8_t max_addresses, uint8_t *actual_found, uint32_t timeout_per_addr_ms);

#endif /* HAL_I2C_H */
