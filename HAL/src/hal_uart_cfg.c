/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_UART_Cfg.c
 * ============================================================================*/
/**
 * @file HAL_UART_Cfg.c
 * @brief Implements the static array of UART configuration settings.
 * This file defines the initial baud rate, parity, stop bits, and pin assignments for UART.
 * It does not contain any initialization functions; its purpose is purely
 * to hold configuration data.
 */

#include "hal_uart_cfg.h"   // Header for UART configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions (UART pins)
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined UART configurations.
 * This array is made `const` and global (`extern` in header) to be accessed by `HAL_UART.c`.
 */
const uart_cfg_item_t s_uart_configurations[] = {
    {
        .uart_num = UART_NUM_0, // ESP32's default console UART
        .config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
        },
        .tx_io_num = HW_UART0_TX_GPIO, // Typically GPIO1
        .rx_io_num = HW_UART0_RX_GPIO, // Typically GPIO3
        .rts_io_num = UART_PIN_NO_CHANGE, // No hardware flow control
        .cts_io_num = UART_PIN_NO_CHANGE, // No hardware flow control
        .rx_buffer_size = 256,
        .tx_buffer_size = 0, // No TX buffer needed for console output
        .event_queue_size = 0, // No event queue needed for console output
    },
    // Example for another UART port (e.g., for external sensor or communication module)
    // {
    //     .uart_num = UART_NUM_1,
    //     .config = {
    //         .baud_rate = 9600,
    //         .data_bits = UART_DATA_8_BITS,
    //         .parity = UART_PARITY_DISABLE,
    //         .stop_bits = UART_STOP_BITS_1,
    //         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    //         .source_clk = UART_SCLK_APB,
    //     },
    //     .tx_io_num = HW_UART1_TX_GPIO_UNUSED, // Define in HAL_Config.h (e.g., GPIO10)
    //     .rx_io_num = HW_UART1_RX_GPIO_UNUSED, // Define in HAL_Config.h (e.g., GPIO9)
    //     .rts_io_num = UART_PIN_NO_CHANGE,
    //     .cts_io_num = UART_PIN_NO_CHANGE,
    //     .rx_buffer_size = 512, // Larger buffer for incoming data
    //     .tx_buffer_size = 1024, // Buffer for outgoing data
    //     .event_queue_size = 10, // For handling UART events like RX_FIFO_OVF, etc.
    // },
};

/**
 * @brief Defines the number of elements in the `s_uart_configurations` array.
 * This variable is made `const` and global (`extern` in header) to be accessed by `HAL_UART.c`.
 */
const size_t s_num_uart_configurations = sizeof(s_uart_configurations) / sizeof(s_uart_configurations[0]);
