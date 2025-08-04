/**
 * @file modbus_cfg.h
 * @brief Service Layer for Modbus RTU Master - Configuration Header.
 *
 * This header defines the compile-time configuration parameters for each
 * Modbus RTU UART port. These settings are typically defined in `modbus_cfg.c`
 * and are used during the initialization of the Modbus service.
 */

#ifndef MODBUS_CFG_H
#define MODBUS_CFG_H

#include <stdint.h>
#include "modbus.h" // Include the main Modbus service interface for enums and types

/**
 * @brief Enumeration for standard UART baud rates.
 */
typedef enum
{
    MODBUS_BAUD_RATE_1200   = 1200,
    MODBUS_BAUD_RATE_2400   = 2400,
    MODBUS_BAUD_RATE_4800   = 4800,
    MODBUS_BAUD_RATE_9600   = 9600,
    MODBUS_BAUD_RATE_19200  = 19200,
    MODBUS_BAUD_RATE_38400  = 38400,
    MODBUS_BAUD_RATE_57600  = 57600,
    MODBUS_BAUD_RATE_115200 = 115200
} MODBUS_BaudRate_t;

/**
 * @brief Enumeration for UART data bits.
 */
typedef enum
{
    MODBUS_DATA_BITS_8 = 8,
    MODBUS_DATA_BITS_7 = 7
} MODBUS_DataBits_t;

/**
 * @brief Enumeration for UART stop bits.
 */
typedef enum
{
    MODBUS_STOP_BITS_1 = 1,
    MODBUS_STOP_BITS_2 = 2
} MODBUS_StopBits_t;

/**
 * @brief Enumeration for UART parity.
 */
typedef enum
{
    MODBUS_PARITY_NONE = 0,
    MODBUS_PARITY_EVEN,
    MODBUS_PARITY_ODD
} MODBUS_Parity_t;

/**
 * @brief Structure to hold compile-time configuration for a single Modbus RTU UART port.
 *
 * This structure defines various parameters for the UART communication,
 * including pin assignments, baud rate, data format, and communication timeouts.
 */
typedef struct
{
    MODBUS_UartPort_t port_id;              /**< The logical ID of the UART port. */
    int tx_gpio_num;                        /**< GPIO pin number for UART TX. */
    int rx_gpio_num;                        /**< GPIO pin number for UART RX. */
    int rts_gpio_num;                       /**< GPIO pin number for RS485 RTS (DE) control. Set to -1 if not used. */
    MODBUS_BaudRate_t baud_rate;            /**< UART baud rate. */
    MODBUS_DataBits_t data_bits;            /**< UART data bits (7 or 8). */
    MODBUS_StopBits_t stop_bits;            /**< UART stop bits (1 or 2). */
    MODBUS_Parity_t parity;                 /**< UART parity (None, Even, Odd). */
    uint32_t response_timeout_ms;           /**< Default timeout for a Modbus response in milliseconds. */
    uint8_t max_retries;                    /**< Maximum number of retries for a Modbus request. */
    size_t rx_buffer_size;                  /**< UART RX buffer size in bytes. */
    size_t tx_buffer_size;                  /**< UART TX buffer size in bytes. */
} MODBUS_PortConfig_t;

/**
 * @brief External declaration for the global array of Modbus port configurations.
 *
 * This array is defined in `modbus_cfg.c` and provides the static
 * configuration for all available Modbus UART ports.
 */
extern const MODBUS_PortConfig_t g_modbus_port_configs[MODBUS_UART_PORT_MAX];

#endif /* MODBUS_CFG_H */
