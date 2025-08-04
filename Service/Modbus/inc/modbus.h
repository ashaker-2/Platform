/**
 * @file modbus.h
 * @brief Service Layer for Modbus RTU Master - Interface Header.
 *
 * This header defines the public API for the Modbus RTU Master service,
 * providing high-level functions to communicate with Modbus slave devices
 * over a serial interface (e.g., UART). It abstracts the Modbus protocol
 * details, allowing applications to read/write registers and coils.
 */

#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for Modbus Service status codes.
 */
typedef enum
{
    MODBUS_STATUS_OK = 0,               /**< Operation successful. */
    MODBUS_STATUS_ERROR,                /**< Generic error. */
    MODBUS_STATUS_INVALID_PARAM,        /**< Invalid parameter provided. */
    MODBUS_STATUS_NOT_INITIALIZED,      /**< Modbus service not initialized. */
    MODBUS_STATUS_ALREADY_INITIALIZED,  /**< Modbus service already initialized. */
    MODBUS_STATUS_BUSY,                 /**< Modbus communication is busy. */
    MODBUS_STATUS_TIMEOUT,              /**< Slave response timeout. */
    MODBUS_STATUS_CRC_ERROR,            /**< CRC mismatch in response. */
    MODBUS_STATUS_ILLEGAL_FUNCTION,     /**< Modbus Exception: Illegal Function. */
    MODBUS_STATUS_ILLEGAL_DATA_ADDRESS, /**< Modbus Exception: Illegal Data Address. */
    MODBUS_STATUS_ILLEGAL_DATA_VALUE,   /**< Modbus Exception: Illegal Data Value. */
    MODBUS_STATUS_SLAVE_DEVICE_FAILURE, /**< Modbus Exception: Slave Device Failure. */
    MODBUS_STATUS_ACKNOWLEDGE,          /**< Modbus Exception: Acknowledge. */
    MODBUS_STATUS_SLAVE_BUSY,           /**< Modbus Exception: Slave Device Busy. */
    MODBUS_STATUS_GATEWAY_PATH_UNAVAILABLE, /**< Modbus Exception: Gateway Path Unavailable. */
    MODBUS_STATUS_GATEWAY_TARGET_NO_RESPONSE, /**< Modbus Exception: Gateway Target Device Failed to Respond. */
    MODBUS_STATUS_UNEXPECTED_RESPONSE,  /**< Response received, but not as expected (e.g., wrong function code). */
    MODBUS_STATUS_MAX                   /**< Maximum number of status codes. */
} MODBUS_Status_t;

/**
 * @brief Enumeration for Modbus RTU UART ports.
 *
 * These map to underlying hardware UART controllers (e.g., UART_NUM_0, UART_NUM_1, UART_NUM_2 on ESP32).
 */
typedef enum
{
    MODBUS_UART_PORT_0 = 0,         /**< UART Port 0. */
    MODBUS_UART_PORT_1,             /**< UART Port 1. */
    MODBUS_UART_PORT_2,             /**< UART Port 2. */
    MODBUS_UART_PORT_MAX            /**< Maximum number of UART ports for Modbus. */
} MODBUS_UartPort_t;

/**
 * @brief Initializes the Modbus RTU master service.
 *
 * This function sets up the underlying UART communication and initializes
 * internal resources for Modbus communication. It must be called before
 * any other Modbus operations.
 *
 * @param port The UART port to be used for Modbus communication.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_Init(MODBUS_UartPort_t port);

/**
 * @brief De-initializes the Modbus RTU master service.
 *
 * This function frees resources associated with the Modbus service and
 * de-initializes the underlying UART driver.
 *
 * @param port The UART port to de-initialize.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_DeInit(MODBUS_UartPort_t port);

/**
 * @brief Reads multiple holding registers from a Modbus slave. (Function Code 0x03)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param start_addr The starting address of the holding registers (0-65535).
 * @param num_regs The number of holding registers to read (1-125).
 * @param data_buffer Pointer to a buffer to store the read 16-bit register values.
 * The buffer must be large enough to hold `num_regs * sizeof(uint16_t)`.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_ReadHoldingRegisters(MODBUS_UartPort_t port,
                                                    uint8_t slave_addr,
                                                    uint16_t start_addr,
                                                    uint16_t num_regs,
                                                    uint16_t *data_buffer);

/**
 * @brief Reads multiple input registers from a Modbus slave. (Function Code 0x04)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param start_addr The starting address of the input registers (0-65535).
 * @param num_regs The number of input registers to read (1-125).
 * @param data_buffer Pointer to a buffer to store the read 16-bit register values.
 * The buffer must be large enough to hold `num_regs * sizeof(uint16_t)`.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_ReadInputRegisters(MODBUS_UartPort_t port,
                                                  uint8_t slave_addr,
                                                  uint16_t start_addr,
                                                  uint16_t num_regs,
                                                  uint16_t *data_buffer);

/**
 * @brief Writes a single holding register to a Modbus slave. (Function Code 0x06)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param reg_addr The address of the holding register to write (0-65535).
 * @param value The 16-bit value to write to the register.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_WriteSingleRegister(MODBUS_UartPort_t port,
                                                   uint8_t slave_addr,
                                                   uint16_t reg_addr,
                                                   uint16_t value);

/**
 * @brief Writes multiple holding registers to a Modbus slave. (Function Code 0x10)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param start_addr The starting address of the holding registers (0-65535).
 * @param num_regs The number of holding registers to write (1-123).
 * @param data_buffer Pointer to a buffer containing the 16-bit values to write.
 * The buffer must contain `num_regs` values.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_WriteMultipleRegisters(MODBUS_UartPort_t port,
                                                      uint8_t slave_addr,
                                                      uint16_t start_addr,
                                                      uint16_t num_regs,
                                                      const uint16_t *data_buffer);

/**
 * @brief Reads multiple coils from a Modbus slave. (Function Code 0x01)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param start_addr The starting address of the coils (0-65535).
 * @param num_coils The number of coils to read (1-2000).
 * @param data_buffer Pointer to a byte buffer to store the read coil states.
 * Each bit in the buffer represents a coil (LSB first).
 * The buffer must be large enough to hold `ceil(num_coils / 8.0)` bytes.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_ReadCoils(MODBUS_UartPort_t port,
                                         uint8_t slave_addr,
                                         uint16_t start_addr,
                                         uint16_t num_coils,
                                         uint8_t *data_buffer);

/**
 * @brief Writes a single coil to a Modbus slave. (Function Code 0x05)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param coil_addr The address of the coil to write (0-65535).
 * @param state The state to write to the coil (true for ON, false for OFF).
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_WriteSingleCoil(MODBUS_UartPort_t port,
                                               uint8_t slave_addr,
                                               uint16_t coil_addr,
                                               bool state);

/**
 * @brief Writes multiple coils to a Modbus slave. (Function Code 0x0F)
 *
 * @param port The UART port configured for Modbus.
 * @param slave_addr The Modbus slave address (1-247).
 * @param start_addr The starting address of the coils (0-65535).
 * @param num_coils The number of coils to write (1-1968).
 * @param data_buffer Pointer to a byte buffer containing the coil states to write.
 * Each bit in the buffer represents a coil (LSB first).
 * The buffer must contain `ceil(num_coils / 8.0)` bytes.
 * @return MODBUS_STATUS_OK if successful, an error code otherwise.
 */
MODBUS_Status_t MODBUS_WriteMultipleCoils(MODBUS_UartPort_t port,
                                                  uint8_t slave_addr,
                                                  uint16_t start_addr,
                                                  uint16_t num_coils,
                                                  const uint8_t *data_buffer);

#endif /* MODBUS_H */
