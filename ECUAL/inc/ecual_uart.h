// ecual/inc/ecual_uart.h

#ifndef ECUAL_UART_H
#define ECUAL_UART_H

#include <stdint.h> // For uint8_t, uint16_t, uint32_t

// IMPORTANT: Directly include the MCAL driver header to align enum values.
// This ties the ECUAL enum values directly to the specific MCU's driver,
// simplifying ecual_uart.c at the cost of less strict ECUAL enum portability.
#include "driver/uart.h" // Provides UART_NUM_0, UART_DATA_8_BITS, UART_STOP_BITS_1, etc.

/**
 * @brief Defines the UART peripheral units.
 * Values are directly aligned with ESP-IDF's uart_port_t.
 */
typedef enum {
    ECUAL_UART_UNIT_0 = UART_NUM_0, ///< UART0 (usually console UART)
    ECUAL_UART_UNIT_1 = UART_NUM_1, ///< UART1
    ECUAL_UART_UNIT_2 = UART_NUM_2  ///< UART2
} ECUAL_UART_Unit_t;

/**
 * @brief Defines the number of data bits.
 * Values are directly aligned with ESP-IDF's uart_word_length_t.
 */
typedef enum {
    ECUAL_UART_DATA_7_BITS = UART_DATA_7_BITS, ///< 7 data bits
    ECUAL_UART_DATA_8_BITS = UART_DATA_8_BITS  ///< 8 data bits
} ECUAL_UART_DataBits_t;

/**
 * @brief Defines the number of stop bits.
 * Values are directly aligned with ESP-IDF's uart_stop_bits_t.
 */
typedef enum {
    ECUAL_UART_STOP_BITS_1   = UART_STOP_BITS_1,    ///< 1 stop bit
    ECUAL_UART_STOP_BITS_1_5 = UART_STOP_BITS_1_5,  ///< 1.5 stop bits
    ECUAL_UART_STOP_BITS_2   = UART_STOP_BITS_2     ///< 2 stop bits
} ECUAL_UART_StopBits_t;

/**
 * @brief Defines the parity mode.
 * Values are directly aligned with ESP-IDF's uart_parity_t.
 */
typedef enum {
    ECUAL_UART_PARITY_DISABLE = UART_PARITY_DISABLE, ///< No parity
    ECUAL_UART_PARITY_EVEN    = UART_PARITY_EVEN,    ///< Even parity
    ECUAL_UART_PARITY_ODD     = UART_PARITY_ODD      ///< Odd parity
} ECUAL_UART_Parity_t;

/**
 * @brief Defines the hardware flow control mode.
 * Values are directly aligned with ESP-IDF's uart_hw_flowcontrol_t.
 */
typedef enum {
    ECUAL_UART_FLOWCONTROL_DISABLE = UART_HW_FLOWCTRL_DISABLE, ///< No hardware flow control
    ECUAL_UART_FLOWCONTROL_RTS_CTS = UART_HW_FLOWCTRL_CTS_RTS  ///< Hardware RTS and CTS flow control
    // You can add more specific RTS/CTS options if needed (UART_HW_FLOWCTRL_RTS, UART_HW_FLOWCTRL_CTS)
} ECUAL_UART_FlowControl_t;

/**
 * @brief Structure to hold the configuration for a single UART unit.
 */
typedef struct {
    ECUAL_UART_Unit_t        unit;          ///< UART peripheral unit (0, 1, or 2)
    int                      baud_rate;     ///< Baud rate (e.g., 9600, 115200)
    ECUAL_UART_DataBits_t    data_bits;     ///< Number of data bits
    ECUAL_UART_StopBits_t    stop_bits;     ///< Number of stop bits
    ECUAL_UART_Parity_t      parity;        ///< Parity mode
    ECUAL_UART_FlowControl_t flow_control;  ///< Hardware flow control mode
    int                      tx_pin;        ///< GPIO pin for TX (e.g., 1 for UART0 TX)
    int                      rx_pin;        ///< GPIO pin for RX (e.g., 3 for UART0 RX)
    int                      rts_pin;       ///< GPIO pin for RTS (-1 if not used)
    int                      cts_pin;       ///< GPIO pin for CTS (-1 if not used)
    uint16_t                 rx_buffer_size; ///< Receive ring buffer size in bytes
    uint16_t                 tx_buffer_size; ///< Transmit ring buffer size in bytes
} ECUAL_UART_Config_t;

/**
 * @brief Initializes all UART units based on the configurations defined in ecual_uart_config.h.
 * @return ECUAL_OK if all units are initialized successfully, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_UART_Init(void);

/**
 * @brief Writes data to a specified UART unit.
 * This function will block until all data is sent or a timeout occurs.
 * @param unit The UART unit to write to.
 * @param data Pointer to the data buffer.
 * @param len The number of bytes to write.
 * @return ECUAL_OK if all bytes were written, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_UART_Write(ECUAL_UART_Unit_t unit, const uint8_t *data, uint16_t len);

/**
 * @brief Reads data from a specified UART unit.
 * This function will attempt to read up to `max_len` bytes. It is non-blocking.
 * @param unit The UART unit to read from.
 * @param data Pointer to the buffer to store read data.
 * @param max_len The maximum number of bytes to read.
 * @return The number of bytes actually read, or 0 if no data is available or an error occurred.
 */
uint16_t ECUAL_UART_Read(ECUAL_UART_Unit_t unit, uint8_t *data, uint16_t max_len);

/**
 * @brief Flushes the UART receive buffer.
 * Clears any unread data from the RX buffer.
 * @param unit The UART unit to flush.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_UART_FlushRx(ECUAL_UART_Unit_t unit);

#endif /* ECUAL_UART_H */
