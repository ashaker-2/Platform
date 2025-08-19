/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_UART.h
 * ============================================================================*/
/**
 * @file HAL_UART.h
 * @brief Public API for interacting with UART peripherals.
 * This header declares functions for initializing the UART, sending, and receiving data.
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include "common.h"  // For Status_t
#include "driver/uart.h"    // For uart_port_t

/**
 * @brief Initializes and configures all UART peripherals according to configurations
 * defined in `HAL_UART_Cfg.c`. This is the main initialization function for the UART HAL.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_UART_Init(void);

/**
 * @brief Writes data to the specified UART port.
 * @param uart_num The UART port number.
 * @param data Pointer to the data buffer to write.
 * @param len The number of bytes to write.
 * @return Number of bytes written on success, or an error code.
 */
int HAL_UART_WriteBytes(uart_port_t uart_num, const char *data, size_t len);

/**
 * @brief Reads data from the specified UART port.
 * @param uart_num The UART port number.
 * @param data_buffer Pointer to the buffer to store read data.
 * @param max_len The maximum number of bytes to read.
 * @param timeout_ms Timeout in milliseconds to wait for data.
 * @return Number of bytes read on success, or 0 if no data available within timeout.
 */
int HAL_UART_ReadBytes(uart_port_t uart_num, uint8_t *data_buffer, size_t max_len, uint32_t timeout_ms);

/**
 * @brief Flushes the UART RX buffer.
 * @param uart_num The UART port number.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_UART_FlushRx(uart_port_t uart_num);

#endif /* HAL_UART_H */
