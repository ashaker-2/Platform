/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_UART.c
 * ============================================================================*/
/**
 * @file HAL_UART.c
 * @brief Implements the public API functions for UART operations,
 * including the module's initialization function.
 * These functions wrap the ESP-IDF UART driver calls with a common status return.
 */

#include "hal_uart.h"       // Header for HAL_UART functions
#include "hal_uart_cfg.h"   // To access UART configuration array
#include "esp_log.h"        // ESP-IDF logging library
#include "driver/uart.h"    // ESP-IDF UART driver
#include "esp_err.h"        // For ESP_OK, ESP_FAIL etc.
#include "freertos/FreeRTOS.h" // For pdMS_TO_TICKS
#include "freertos/task.h"   // For pdMS_TO_TICKS

static const char *TAG = "HAL_UART";

/**
 * @brief Initializes and configures all UART peripherals according to configurations
 * defined in the internal `s_uart_configurations` array from `HAL_UART_Cfg.c`.
 *
 * @return E_OK on success, or an error code.
 */
Status_t HAL_UART_Init(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying UART configurations from HAL_UART_Cfg.c...");

    for (size_t i = 0; i < s_num_uart_configurations; i++) {
        const uart_cfg_item_t *cfg_item = &s_uart_configurations[i];

        // Configure UART parameters
        ret = uart_param_config(cfg_item->uart_num, &cfg_item->config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "UART%d param config failed: %s", cfg_item->uart_num, esp_err_to_name(ret));
            return E_ERROR;
        }

        // Set UART pins
        ret = uart_set_pin(cfg_item->uart_num,
                           cfg_item->tx_io_num,
                           cfg_item->rx_io_num,
                           cfg_item->rts_io_num,
                           cfg_item->cts_io_num);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "UART%d pin set failed: %s", cfg_item->uart_num, esp_err_to_name(ret));
            return E_ERROR;
        }

        // Install UART driver with buffer sizes and event queue (if applicable)
        ret = uart_driver_install(cfg_item->uart_num,
                                  cfg_item->rx_buffer_size,
                                  cfg_item->tx_buffer_size,
                                  cfg_item->event_queue_size,
                                  NULL, // No event queue handle needed if queue_size is 0
                                  0);   // No ISR flags
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "UART%d driver install failed: %s", cfg_item->uart_num, esp_err_to_name(ret));
            return E_ERROR;
        }

        ESP_LOGD(TAG, "UART%d initialized: Baud=%ld, TX=%d, RX=%d",
                 cfg_item->uart_num, cfg_item->config.baud_rate, cfg_item->tx_io_num, cfg_item->rx_io_num);
    }

    ESP_LOGI(TAG, "All UARTs initialized successfully.");
    return E_OK;
}

/**
 * @brief Writes data to the specified UART port.
 * @param uart_num The UART port number.
 * @param data Pointer to the data buffer to write.
 * @param len The number of bytes to write.
 * @return Number of bytes written on success, or an error code.
 */
int HAL_UART_WriteBytes(uart_port_t uart_num, const char *data, size_t len) {
    int written_bytes = uart_write_bytes(uart_num, data, len);
    if (written_bytes < 0) {
        ESP_LOGE(TAG, "UART%d write failed.", uart_num);
        return E_ERROR; // Returning E_ERROR for consistency, though uart_write_bytes returns -1 on error
    }
    return written_bytes;
}

/**
 * @brief Reads data from the specified UART port.
 * @param uart_num The UART port number.
 * @param data_buffer Pointer to the buffer to store read data.
 * @param max_len The maximum number of bytes to read.
 * @param timeout_ms Timeout in milliseconds to wait for data.
 * @return Number of bytes read on success, or 0 if no data available within timeout.
 */
int HAL_UART_ReadBytes(uart_port_t uart_num, uint8_t *data_buffer, size_t max_len, uint32_t timeout_ms) {
    if (data_buffer == NULL) {
        ESP_LOGE(TAG, "HAL_UART_ReadBytes: data_buffer is NULL for UART%d.", uart_num);
        return E_INVALID_PARAM;
    }
    int read_bytes = uart_read_bytes(uart_num, data_buffer, max_len, pdMS_TO_TICKS(timeout_ms));
    if (read_bytes < 0) {
        ESP_LOGE(TAG, "UART%d read failed.", uart_num);
        return E_ERROR;
    }
    return read_bytes;
}

/**
 * @brief Flushes the UART RX buffer.
 * @param uart_num The UART port number.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_UART_FlushRx(uart_port_t uart_num) {
    esp_err_t ret = uart_flush_input(uart_num);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART%d RX flush failed: %s", uart_num, esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "UART%d RX buffer flushed.", uart_num);
    return E_OK;
}
