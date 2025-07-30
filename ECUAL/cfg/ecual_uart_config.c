// ecual/src/ecual_uart_config.c

#include "ecual_uart_config.h" // Includes ecual_uart.h for ECUAL_UART_Config_t definition
#include "driver/gpio.h" // For GPIO_NUM_X definitions, if you want to use them directly

// Definition of the static constant array for UART channel configurations
const ECUAL_UART_Config_t uart_channel_configurations[] = {
    // UART0 for Debug Console
    {
        .unit = ECUAL_UART_DEBUG_CONSOLE,
        .baud_rate = 115200,
        .data_bits = ECUAL_UART_DATA_8_BITS,
        .stop_bits = ECUAL_UART_STOP_BITS_1,
        .parity = ECUAL_UART_PARITY_DISABLE,
        .flow_control = ECUAL_UART_FLOWCONTROL_DISABLE,
        .tx_pin = GPIO_NUM_1,  // Default TX pin for UART0
        .rx_pin = GPIO_NUM_3,  // Default RX pin for UART0
        .rts_pin = UART_PIN_NO_CHANGE, // Use -1 or UART_PIN_NO_CHANGE
        .cts_pin = UART_PIN_NO_CHANGE,
        .rx_buffer_size = 256,
        .tx_buffer_size = 0, // No dedicated TX buffer for console, will use direct write
    },
    // UART1 for GPS Module
    {
        .unit = ECUAL_UART_GPS_MODULE,
        .baud_rate = 9600,
        .data_bits = ECUAL_UART_DATA_8_BITS,
        .stop_bits = ECUAL_UART_STOP_BITS_1,
        .parity = ECUAL_UART_PARITY_DISABLE,
        .flow_control = ECUAL_UART_FLOWCONTROL_DISABLE,
        .tx_pin = GPIO_NUM_10, // Example custom pin
        .rx_pin = GPIO_NUM_9,  // Example custom pin
        .rts_pin = UART_PIN_NO_CHANGE,
        .cts_pin = UART_PIN_NO_CHANGE,
        .rx_buffer_size = 512, // Larger RX buffer for GPS data
        .tx_buffer_size = 256,
    },
    // UART2 for Bluetooth Module with Flow Control
    {
        .unit = ECUAL_UART_BLUETOOTH_MODULE,
        .baud_rate = 115200,
        .data_bits = ECUAL_UART_DATA_8_BITS,
        .stop_bits = ECUAL_UART_STOP_BITS_1,
        .parity = ECUAL_UART_PARITY_DISABLE,
        .flow_control = ECUAL_UART_FLOWCONTROL_RTS_CTS,
        .tx_pin = GPIO_NUM_17,
        .rx_pin = GPIO_NUM_16,
        .rts_pin = GPIO_NUM_18, // Example custom pin for RTS
        .cts_pin = GPIO_NUM_19, // Example custom pin for CTS
        .rx_buffer_size = 1024,
        .tx_buffer_size = 1024,
    }
};

// Definition of the number of configurations in the array
const uint32_t ECUAL_NUM_UART_CONFIGURATIONS = sizeof(uart_channel_configurations) / sizeof(ECUAL_UART_Config_t);
