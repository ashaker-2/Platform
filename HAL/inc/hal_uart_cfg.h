/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/hal_uart_cfg.h
 * ============================================================================*/
/**
 * @file hal_uart_cfg.h
 * @brief Declarations for external UART configuration data.
 * This header makes the static UART configuration array and parameters available
 * to HAL_UART.c for initialization. It does not declare any functions.
 */
#ifndef HAL_UART_CFG_H
#define HAL_UART_CFG_H

#include "driver/uart.h"    // For uart_port_t, uart_config_t
#include <stddef.h>         // For size_t

/**
 * @brief Structure to hold UART configuration along with its port ID.
 * Defined here for external visibility, used in HAL_UART_Cfg.c.
 */
typedef struct {
    uart_port_t uart_num;
    uart_config_t config;
    int tx_io_num;
    int rx_io_num;
    int rts_io_num;
    int cts_io_num;
    int rx_buffer_size;
    int tx_buffer_size;
    int event_queue_size;
} uart_cfg_item_t;

/**
 * @brief External declaration of the array containing all predefined UART configurations.
 * This array is defined in HAL_UART_Cfg.c and accessed by HAL_UART.c to perform
 * initial UART setup.
 */
extern const uart_cfg_item_t s_uart_configurations[];

/**
 * @brief External declaration of the number of elements in the UART configurations array.
 */
extern const size_t s_num_uart_configurations;

#endif /* HAL_UART_CFG_H */
