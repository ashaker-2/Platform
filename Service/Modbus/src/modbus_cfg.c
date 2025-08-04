/**
 * @file modbus_cfg.c
 * @brief Service Layer for Modbus RTU Master - Configuration Implementation.
 *
 * This file implements the static configuration parameters for each Modbus RTU UART port.
 * These settings are used by the `modbus.c` implementation to initialize
 * and operate the Modbus master functionalities.
 *
 * IMPORTANT: Adjust these values according to your specific application's
 * hardware connections and desired Modbus communication parameters.
 */

#include "modbus_cfg.h"
#include "modbus.h" // For MODBUS_UartPort_t and other enums

// Define the global array of Modbus port configurations
const MODBUS_PortConfig_t g_modbus_port_configs[MODBUS_UART_PORT_MAX] = {
    [MODBUS_UART_PORT_0] = {
        .port_id = MODBUS_UART_PORT_0,
        .tx_gpio_num = 17,                      // Example: GPIO 17 for UART0 TX
        .rx_gpio_num = 16,                      // Example: GPIO 16 for UART0 RX
        .rts_gpio_num = -1,                     // No RTS (DE) control by default for UART0, typically used for console
        .baud_rate = MODBUS_BAUD_RATE_115200, // Default to 115200 baud
        .data_bits = MODBUS_DATA_BITS_8,
        .stop_bits = MODBUS_STOP_BITS_1,
        .parity = MODBUS_PARITY_NONE,
        .response_timeout_ms = 500,             // 500ms response timeout
        .max_retries = 3,                       // 3 retries on failure
        .rx_buffer_size = 256,                  // UART RX buffer size
        .tx_buffer_size = 128,                  // UART TX buffer size
    },
    [MODBUS_UART_PORT_1] = {
        .port_id = MODBUS_UART_PORT_1,
        .tx_gpio_num = 4,                       // Example: GPIO 4 for UART1 TX
        .rx_gpio_num = 5,                       // Example: GPIO 5 for UART1 RX
        .rts_gpio_num = 2,                      // Example: GPIO 2 for RS485 DE/RE control
        .baud_rate = MODBUS_BAUD_RATE_9600, // Default to 9600 baud
        .data_bits = MODBUS_DATA_BITS_8,
        .stop_bits = MODBUS_STOP_BITS_1,
        .parity = MODBUS_PARITY_EVEN,       // Even parity for Modbus RTU
        .response_timeout_ms = 1000,            // 1000ms response timeout
        .max_retries = 2,                       // 2 retries on failure
        .rx_buffer_size = 512,                  // Larger RX buffer for potential longer responses
        .tx_buffer_size = 256,                  // Larger TX buffer
    },
    [MODBUS_UART_PORT_2] = {
        .port_id = MODBUS_UART_PORT_2,
        .tx_gpio_num = 18,                      // Example: GPIO 18 for UART2 TX
        .rx_gpio_num = 19,                      // Example: GPIO 19 for UART2 RX
        .rts_gpio_num = 21,                     // Example: GPIO 21 for RS485 DE/RE control
        .baud_rate = MODBUS_BAUD_RATE_19200, // Default to 19200 baud
        .data_bits = MODBUS_DATA_BITS_8,
        .stop_bits = MODBUS_STOP_BITS_1,
        .parity = MODBUS_PARITY_NONE,
        .response_timeout_ms = 750,             // 750ms response timeout
        .max_retries = 1,                       // 1 retry on failure
        .rx_buffer_size = 256,
        .tx_buffer_size = 128,
    },
    // Add configurations for more UART ports if defined in modbus.h
};

