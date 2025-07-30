// ecual/src/ecual_uart.c

#include "ecual_uart.h"        // Includes driver/uart.h
#include "ecual_uart_config.h" // For uart_channel_configurations array extern declaration
#include "ecual_common.h"      // For ECUAL_OK and ECUAL_ERROR

// Implementation for UART initialization and operations

uint8_t ECUAL_UART_Init(void) {
    uint32_t i;
    esp_err_t mcal_ret;
    uint8_t ecual_ret = ECUAL_OK;

    for (i = 0; i < ECUAL_NUM_UART_CONFIGURATIONS; i++) {
        const ECUAL_UART_Config_t *cfg = &uart_channel_configurations[i];

        // 1. Configure UART parameters
        uart_config_t uart_config = {
            .baud_rate = cfg->baud_rate,
            .data_bits = (uart_word_length_t)cfg->data_bits, // Direct cast
            .parity = (uart_parity_t)cfg->parity,           // Direct cast
            .stop_bits = (uart_stop_bits_t)cfg->stop_bits,   // Direct cast
            .flow_ctrl = (uart_hw_flowcontrol_t)cfg->flow_control, // Direct cast
            .source_clk = UART_SCLK_APB, // Use APB clock
        };

        mcal_ret = uart_param_config((uart_port_t)cfg->unit, &uart_config);
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            // Optionally: add logging here for which UART unit failed
            continue;
        }

        // 2. Install UART driver (with RX/TX ring buffers, no event queue for this simple ECUAL)
        mcal_ret = uart_driver_install((uart_port_t)cfg->unit, cfg->rx_buffer_size, cfg->tx_buffer_size, 0, NULL, 0);
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            continue;
        }

        // 3. Set UART pins
        mcal_ret = uart_set_pin(
            (uart_port_t)cfg->unit,
            cfg->tx_pin,
            cfg->rx_pin,
            cfg->rts_pin,
            cfg->cts_pin
        );
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            continue;
        }
    }

    return ecual_ret;
}

uint8_t ECUAL_UART_Write(ECUAL_UART_Unit_t unit, const uint8_t *data, uint16_t len) {
    // uart_write_bytes returns the number of bytes written, or -1 on error
    int bytes_written = uart_write_bytes((uart_port_t)unit, (const char *)data, len);

    // Check if all bytes were written (or if an error occurred)
    if (bytes_written == len) {
        return ECUAL_OK;
    } else {
        return ECUAL_ERROR;
    }
}

uint16_t ECUAL_UART_Read(ECUAL_UART_Unit_t unit, uint8_t *data, uint16_t max_len) {
    // uart_read_bytes returns number of bytes read, or -1 on error.
    // Timeout of 0 means non-blocking.
    int bytes_read = uart_read_bytes((uart_port_t)unit, data, max_len, 0);

    if (bytes_read == -1) {
        return 0; // Return 0 bytes read on error
    } else {
        return (uint16_t)bytes_read;
    }
}

uint8_t ECUAL_UART_FlushRx(ECUAL_UART_Unit_t unit) {
    esp_err_t mcal_ret = uart_flush_input((uart_port_t)unit);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}
