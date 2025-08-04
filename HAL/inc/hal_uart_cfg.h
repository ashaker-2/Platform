/**
 * @file hal_uart_cfg.h
 * @brief Hardware Abstraction Layer for UART (Universal Asynchronous Receiver/Transmitter) - Configuration Interface.
 *
 * This module defines the compile-time configuration parameters for each
 * UART channel. It allows for static setup of UART properties like baud rate,
 * data format, and buffer sizes without modifying the core HAL driver logic.
 *
 * The actual configuration values are defined in hal_uart_cfg.c.
 */

#ifndef HAL_UART_CFG_H
#define HAL_UART_CFG_H

#include <stdint.h>
#include <stdbool.h>
#include "hal_uart.h" // Include the main HAL interface for HAL_UART_Channel_t and HAL_UART_RxCpltCallback_t

/**
 * @brief Structure for UART channel-specific configuration.
 *
 * This structure holds all the necessary parameters to initialize a single
 * UART peripheral instance.
 */
typedef struct
{
    HAL_UART_Channel_t channel_id;      /**< Unique ID for the UART channel (e.g., HAL_UART_CHANNEL_0) */
    uint32_t baud_rate;                 /**< Baud rate (e.g., 9600, 115200) */
    uint8_t data_bits;                  /**< Number of data bits (e.g., 7, 8) */
    uint8_t stop_bits;                  /**< Number of stop bits (e.g., 1, 2) */
    bool parity_enable;                 /**< True to enable parity, false otherwise */
    bool even_parity;                   /**< True for even parity, false for odd parity (if parity_enable is true) */
    bool flow_control_enable;           /**< True to enable hardware flow control (RTS/CTS) */
    uint32_t rx_buffer_size;            /**< Size of the receive buffer in bytes (0 for no buffer) */
    uint32_t tx_buffer_size;            /**< Size of the transmit buffer in bytes (0 for no buffer) */
    // Add GPIO pin configurations if the HAL is responsible for pinmuxing
    // uint8_t tx_pin;
    // uint8_t rx_pin;
    // uint8_t rts_pin; // If flow control enabled
    // uint8_t cts_pin; // If flow control enabled

    HAL_UART_RxCpltCallback_t rx_callback; /**< Optional: Pointer to a callback function for received data.
                                                Pass NULL if no callback is needed (e.g., for polling). */
} HAL_UART_ChannelConfig_t;

/**
 * @brief External declaration of the UART channel configurations array.
 *
 * This array will be defined in hal_uart_cfg.c and will contain the
 * configuration for each UART channel used in the system.
 * The size of this array should match HAL_UART_CHANNEL_MAX from hal_uart.h
 * if all channels are to be configured, or a subset if only specific ones are used.
 */
extern const HAL_UART_ChannelConfig_t g_hal_uart_channel_configs[HAL_UART_CHANNEL_MAX];

#endif /* HAL_UART_CFG_H */
