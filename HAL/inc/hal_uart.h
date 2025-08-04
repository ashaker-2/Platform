/**
 * @file hal_uart.h
 * @brief Hardware Abstraction Layer for UART (Universal Asynchronous Receiver/Transmitter) - Interface.
 *
 * This module provides an abstract interface for UART communication, allowing
 * the application layer to send and receive data without direct knowledge
 * of the underlying UART peripheral registers or specific hardware details.
 *
 * It supports basic UART operations like initialization, deinitialization,
 * byte transmission, and byte reception. It also includes mechanisms for
 * interrupt-driven reception.
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for HAL_UART return codes.
 */
typedef enum
{
    HAL_UART_OK = 0,            /**< Operation successful */
    HAL_UART_ERROR,             /**< General error */
    HAL_UART_INVALID_PARAM,     /**< Invalid parameter provided */
    HAL_UART_BUSY,              /**< UART peripheral is currently busy */
    HAL_UART_TIMEOUT,           /**< Operation timed out */
    HAL_UART_NOT_INITIALIZED,   /**< UART module is not initialized */
    HAL_UART_TX_BUFFER_FULL,    /**< Transmit buffer is full */
    HAL_UART_RX_BUFFER_EMPTY    /**< Receive buffer is empty */
} HAL_UART_Status_t;

/**
 * @brief Enumeration for available UART channels/ports.
 *
 * Add more UART ports as per the target microcontroller's capabilities.
 * For ESP32, typically UART0, UART1, UART2.
 */
typedef enum
{
    HAL_UART_CHANNEL_0 = 0,     /**< UART Channel 0 */
    HAL_UART_CHANNEL_1,         /**< UART Channel 1 */
    HAL_UART_CHANNEL_2,         /**< UART Channel 2 */
    // Add more UART channels if available on the target hardware
    HAL_UART_CHANNEL_MAX        /**< Sentinel for maximum number of UART channels */
} HAL_UART_Channel_t;

/**
 * @brief Structure for UART configuration parameters.
 */
typedef struct
{
    uint32_t baud_rate;         /**< Baud rate (e.g., 9600, 115200) */
    uint8_t data_bits;          /**< Number of data bits (e.g., 7, 8) */
    uint8_t stop_bits;          /**< Number of stop bits (e.g., 1, 2) */
    bool parity_enable;         /**< True to enable parity, false otherwise */
    bool even_parity;           /**< True for even parity, false for odd parity (if parity_enable is true) */
    bool flow_control_enable;   /**< True to enable hardware flow control (RTS/CTS) */
    uint32_t rx_buffer_size;    /**< Size of the receive buffer in bytes (0 for no buffer) */
    uint32_t tx_buffer_size;    /**< Size of the transmit buffer in bytes (0 for no buffer) */
} HAL_UART_Config_t;

/**
 * @brief Callback function type for UART receive events.
 *
 * This function will be called by the HAL driver when new data is received
 * on a specific UART channel.
 * @param channel The UART channel on which data was received.
 * @param data The received byte of data.
 */
typedef void (*HAL_UART_RxCpltCallback_t)(HAL_UART_Channel_t channel, uint8_t data);

/**
 * @brief Initializes a specific UART channel with the given configuration.
 *
 * This function sets up the UART peripheral, including baud rate, data format,
 * and enables necessary interrupts.
 * @param channel The UART channel to initialize.
 * @param config Pointer to the configuration structure for the UART channel.
 * @param rx_callback Optional: Pointer to a callback function for received data.
 * Pass NULL if no callback is needed (e.g., for polling).
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_Init(HAL_UART_Channel_t channel, const HAL_UART_Config_t *config, HAL_UART_RxCpltCallback_t rx_callback);

/**
 * @brief De-initializes a specific UART channel.
 *
 * This function disables the UART peripheral and releases any associated resources.
 * @param channel The UART channel to de-initialize.
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_DeInit(HAL_UART_Channel_t channel);

/**
 * @brief Transmits a single byte over the specified UART channel.
 *
 * This function sends one byte of data. It might block until the byte is sent
 * or use a transmit buffer if configured.
 * @param channel The UART channel to transmit on.
 * @param data The byte to transmit.
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_TransmitByte(HAL_UART_Channel_t channel, uint8_t data);

/**
 * @brief Transmits an array of bytes over the specified UART channel.
 *
 * This function sends a buffer of data. It might block until all bytes are sent
 * or use a transmit buffer if configured.
 * @param channel The UART channel to transmit on.
 * @param data_buffer Pointer to the buffer containing data to transmit.
 * @param length The number of bytes to transmit.
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_Transmit(HAL_UART_Channel_t channel, const uint8_t *data_buffer, uint32_t length);

/**
 * @brief Receives a single byte from the specified UART channel.
 *
 * This function attempts to read one byte of data. It might block until a byte
 * is received or return an error if no data is available (for non-blocking mode).
 * @param channel The UART channel to receive from.
 * @param data Pointer to a variable where the received byte will be stored.
 * @param timeout_ms Timeout in milliseconds for blocking read (0 for non-blocking).
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_ReceiveByte(HAL_UART_Channel_t channel, uint8_t *data, uint32_t timeout_ms);

/**
 * @brief Receives an array of bytes from the specified UART channel.
 *
 * This function attempts to read a buffer of data. It might block until all bytes
 * are received or return an error if not enough data is available.
 * @param channel The UART channel to receive from.
 * @param data_buffer Pointer to the buffer where received data will be stored.
 * @param length The number of bytes to receive.
 * @param timeout_ms Timeout in milliseconds for blocking read (0 for non-blocking).
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_Receive(HAL_UART_Channel_t channel, uint8_t *data_buffer, uint32_t length, uint32_t timeout_ms);

/**
 * @brief Flushes the transmit buffer for a given UART channel.
 * @param channel The UART channel.
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_FlushTxBuffer(HAL_UART_Channel_t channel);

/**
 * @brief Flushes the receive buffer for a given UART channel.
 * @param channel The UART channel.
 * @return HAL_UART_OK if successful, otherwise an error code.
 */
HAL_UART_Status_t HAL_UART_FlushRxBuffer(HAL_UART_Channel_t channel);

/**
 * @brief Gets the number of bytes currently in the receive buffer.
 * @param channel The UART channel.
 * @return The number of bytes in the receive buffer.
 */
uint32_t HAL_UART_GetRxBufferSize(HAL_UART_Channel_t channel);

#endif /* HAL_UART_H */
