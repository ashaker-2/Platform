/**
 * @file hal_uart_cfg.c
 * @brief Hardware Abstraction Layer for UART (Universal Asynchronous Receiver/Transmitter) - Configuration Definitions.
 *
 * This file contains the compile-time configuration definitions for each
 * UART channel used in the system. These configurations are used by the
 * HAL_UART_Init function to set up the UART peripherals.
 *
 * Users should modify this file to configure their specific UART channels.
 */

#include "hal_uart_cfg.h"
#include "hal_uart.h" // Include main HAL for channel enums if not already included by cfg.h

// Forward declarations for any UART RX completion callbacks, if used.
// These would typically be implemented in application or higher layers.
// For demonstration, we'll use NULL or simple dummy callbacks.
// void App_UART0_RxCpltCallback(HAL_UART_Channel_t channel, uint8_t received_byte);
// void App_UART1_RxCpltCallback(HAL_UART_Channel_t channel, uint8_t received_byte);

/**
 * @brief Array of UART channel configurations.
 *
 * This array defines the static configuration for each UART channel available
 * in the system. The index of the array corresponds to the HAL_UART_Channel_t enum.
 *
 * IMPORTANT: Ensure the number of entries matches HAL_UART_CHANNEL_MAX.
 * If a channel is not used, its entry can be initialized with default/dummy values,
 * but it must still be present to match the array size.
 */
const HAL_UART_ChannelConfig_t g_hal_uart_channel_configs[HAL_UART_CHANNEL_MAX] =
{
    [HAL_UART_CHANNEL_0] =
    {
        .channel_id        = HAL_UART_CHANNEL_0,
        .baud_rate         = 115200,
        .data_bits         = 8,
        .stop_bits         = 1,
        .parity_enable     = false,
        .even_parity       = false, // Not applicable if parity_enable is false
        .flow_control_enable = false,
        .rx_buffer_size    = 256,   // Example: 256 bytes RX buffer
        .tx_buffer_size    = 128,   // Example: 128 bytes TX buffer
        // .rx_callback       = NULL   // No specific callback for UART0 in this example
        // .tx_pin = GPIO_NUM_1, // Example GPIO pin, uncomment if HAL handles pinmux
        // .rx_pin = GPIO_NUM_3,
    },
    [HAL_UART_CHANNEL_1] =
    {
        .channel_id        = HAL_UART_CHANNEL_1,
        .baud_rate         = 9600,
        .data_bits         = 8,
        .stop_bits         = 1,
        .parity_enable     = true,
        .even_parity       = true, // Even parity
        .flow_control_enable = false,
        .rx_buffer_size    = 128,
        .tx_buffer_size    = 64,
        // .rx_callback       = NULL   // No specific callback for UART1 in this example
        // .tx_pin = GPIO_NUM_10,
        // .rx_pin = GPIO_NUM_9,
    },
    [HAL_UART_CHANNEL_2] =
    {
        .channel_id        = HAL_UART_CHANNEL_2,
        .baud_rate         = 38400,
        .data_bits         = 8,
        .stop_bits         = 1,
        .parity_enable     = false,
        .even_parity       = false,
        .flow_control_enable = true, // Enable hardware flow control
        .rx_buffer_size    = 512,
        .tx_buffer_size    = 256,
        // .rx_callback       = NULL   // No specific callback for UART2 in this example
        // .tx_pin = GPIO_NUM_17,
        // .rx_pin = GPIO_NUM_16,
        // .rts_pin = GPIO_NUM_18,
        // .cts_pin = GPIO_NUM_19,
    },
    // Add more UART channel configurations here if HAL_UART_CHANNEL_MAX is larger
    // and you have more physical UARTs to configure.
};

// Example of a dummy callback function (if you were to assign it)
/*
void App_UART0_RxCpltCallback(HAL_UART_Channel_t channel, uint8_t received_byte)
{
    // This function would be implemented in the application layer
    // and registered with HAL_UART_Init if needed.
    // For example, put the received_byte into a FreeRTOS queue.
    (void)channel; // Unused parameter
    (void)received_byte; // Unused parameter
}
*/
