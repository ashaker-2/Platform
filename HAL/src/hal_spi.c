/**
 * @file hal_spi.c
 * @brief Hardware Abstraction Layer for SPI (Serial Peripheral Interface) - Implementation.
 *
 * This module provides the concrete implementation for the SPI HAL interface
 * defined in `hal_spi.h`. It utilizes the ESP-IDF SPI Master driver to
 * configure and manage SPI communication on the ESP32.
 */

#include "hal_spi.h"
#include "hal_spi_cfg.h.h"     // Include configuration header
#include "driver/spi_master.h" // ESP-IDF SPI driver
#include "driver/gpio.h"       // ESP-IDF GPIO driver
#include "esp_log.h"           // ESP-IDF logging
#include "freertos/FreeRTOS.h" // For FreeRTOS types (e.g., TickType_t)
#include "freertos/task.h"     // For vTaskDelay

// Define a tag for ESP-IDF logging
static const char *TAG = "HAL_SPI";

/**
 * @brief Structure to hold runtime data for each SPI channel.
 */
typedef struct
{
    spi_device_handle_t spi_handle;      /**< Handle to the ESP-IDF SPI device. */
    bool initialized;                    /**< Flag indicating if the channel is initialized. */
    HAL_SPI_TxRxCpltCallback_t callback; /**< Callback function for asynchronous transfers. */
    SemaphoreHandle_t transfer_done_sem; /**< Semaphore to signal completion of asynchronous transfers. */
    HAL_SPI_Status_t async_status;       /**< Status of the last asynchronous transfer. */
} HAL_SPI_ChannelData_t;

// Array to hold runtime data for each SPI channel
static HAL_SPI_ChannelData_t g_spi_channel_data[HAL_SPI_CHANNEL_MAX];

/**
 * @brief Internal callback for ESP-IDF SPI transactions.
 *
 * This function is called by the ESP-IDF SPI driver when an asynchronous
 * transaction completes. It signals a semaphore and invokes the user-defined
 * callback if provided.
 *
 * @param trans The completed SPI transaction.
 */
static void IRAM_ATTR spi_transaction_callback(spi_transaction_t *trans)
{
    HAL_SPI_Channel_t channel = (HAL_SPI_Channel_t)trans->user;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (channel < HAL_SPI_CHANNEL_MAX && g_spi_channel_data[channel].initialized)
    {
        // Set the status for the asynchronous transfer
        g_spi_channel_data[channel].async_status = HAL_SPI_STATUS_OK;

        // Give the semaphore to unblock the waiting task (if any)
        if (g_spi_channel_data[channel].transfer_done_sem != NULL)
        {
            xSemaphoreGiveFromISR(g_spi_channel_data[channel].transfer_done_sem, &xHigherPriorityTaskWoken);
        }

        // Invoke the user-defined callback
        if (g_spi_channel_data[channel].callback != NULL)
        {
            g_spi_channel_data[channel].callback(channel, HAL_SPI_STATUS_OK);
        }
    }
    else
    {
        ESP_LOGE(TAG, "SPI callback received for uninitialized or invalid channel: %d", channel);
    }

    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief Converts HAL_SPI_Mode_t to ESP-IDF SPI_MODE.
 * @param mode The HAL SPI mode.
 * @return The corresponding ESP-IDF SPI_MODE.
 */
static spi_mode_t hal_spi_mode_to_esp_mode(HAL_SPI_Mode_t mode)
{
    switch (mode)
    {
    case HAL_SPI_MODE_0:
        return SPI_MODE0;
    case HAL_SPI_MODE_1:
        return SPI_MODE1;
    case HAL_SPI_MODE_2:
        return SPI_MODE2;
    case HAL_SPI_MODE_3:
        return SPI_MODE3;
    default:
        return SPI_MODE0; // Default to mode 0
    }
}

/**
 * @brief Converts HAL_SPI_Channel_t to ESP-IDF spi_host_device_t.
 * @param channel The HAL SPI channel.
 * @return The corresponding ESP-IDF spi_host_device_t.
 */
static spi_host_device_t hal_spi_channel_to_esp_host(HAL_SPI_Channel_t channel)
{
    switch (channel)
    {
    case HAL_SPI_CHANNEL_0:
        return VSPI_HOST; // Typically SPI3 on ESP32
    case HAL_SPI_CHANNEL_1:
        return HSPI_HOST; // Typically SPI2 on ESP32
    default:
        return SPI1_HOST; // Fallback, though should be caught by validation
    }
}

HAL_SPI_Status_t HAL_SPI_Init(HAL_SPI_Channel_t channel)
{
    if (channel >= HAL_SPI_CHANNEL_MAX)
    {
        ESP_LOGE(TAG, "Invalid SPI channel: %d", channel);
        return HAL_SPI_STATUS_INVALID_CHANNEL;
    }

    if (g_spi_channel_data[channel].initialized)
    {
        ESP_LOGW(TAG, "SPI channel %d already initialized.", channel);
        return HAL_SPI_STATUS_OK;
    }

    const HAL_SPI_ChannelConfig_t *cfg = &g_hal_spi_channel_configs[channel];

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = cfg->mosi_pin,
        .miso_io_num = cfg->miso_pin,
        .sclk_io_num = cfg->sclk_pin,
        .quadwp_io_num = -1,  // Not using Quad SPI
        .quadhd_io_num = -1,  // Not using Quad SPI
        .max_transfer_sz = 0, // 0 means default (4092 bytes)
        .flags = 0,
        .isr_cpu_id = INTR_CPU_ID_AUTO, // Let ESP-IDF decide ISR CPU
    };

    // Configure SPI device
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = hal_spi_mode_to_esp_mode(cfg->spi_mode),
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = (int)cfg->baud_rate_hz,
        .input_delay_ns = 0,
        .spics_io_num = -1, // CS pin is managed externally or by higher layer
        .flags = 0,
        .queue_size = 7,                     // Number of transactions that can be queued
        .pre_cb = NULL,                      // Pre-transfer callback (e.g., for CS assertion)
        .post_cb = spi_transaction_callback, // Post-transfer callback (for IT transfers)
    };

    spi_host_device_t host = hal_spi_channel_to_esp_host(channel);

    // Initialize the SPI bus
    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI bus %d: %s", host, esp_err_to_name(ret));
        return HAL_SPI_STATUS_ERROR;
    }

    // Add the SPI device to the bus
    ret = spi_bus_add_device(host, &devcfg, &g_spi_channel_data[channel].spi_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI device to bus %d: %s", host, esp_err_to_name(ret));
        spi_bus_free(host); // Free the bus if device addition fails
        return HAL_SPI_STATUS_ERROR;
    }

    // Create a semaphore for asynchronous transfer completion
    g_spi_channel_data[channel].transfer_done_sem = xSemaphoreCreateBinary();
    if (g_spi_channel_data[channel].transfer_done_sem == NULL)
    {
        ESP_LOGE(TAG, "Failed to create semaphore for SPI channel %d", channel);
        spi_bus_remove_device(g_spi_channel_data[channel].spi_handle);
        spi_bus_free(host);
        return HAL_SPI_STATUS_ERROR;
    }

    g_spi_channel_data[channel].initialized = true;
    LOGI(TAG, "SPI channel %d (Host %d) initialized successfully. Baud: %lu Hz",
         channel, host, cfg->baud_rate_hz);
    return HAL_SPI_STATUS_OK;
}

HAL_SPI_Status_t HAL_SPI_DeInit(HAL_SPI_Channel_t channel)
{
    if (channel >= HAL_SPI_CHANNEL_MAX)
    {
        ESP_LOGE(TAG, "Invalid SPI channel: %d", channel);
        return HAL_SPI_STATUS_INVALID_CHANNEL;
    }

    if (!g_spi_channel_data[channel].initialized)
    {
        ESP_LOGW(TAG, "SPI channel %d not initialized.", channel);
        return HAL_SPI_STATUS_OK;
    }

    spi_host_device_t host = hal_spi_channel_to_esp_host(channel);
    esp_err_t ret;

    // Remove the device from the bus
    ret = spi_bus_remove_device(g_spi_channel_data[channel].spi_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to remove SPI device from bus %d: %s", host, esp_err_to_name(ret));
        return HAL_SPI_STATUS_ERROR;
    }

    // Free the SPI bus
    ret = spi_bus_free(host);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to free SPI bus %d: %s", host, esp_err_to_name(ret));
        return HAL_SPI_STATUS_ERROR;
    }

    // Delete the semaphore
    if (g_spi_channel_data[channel].transfer_done_sem != NULL)
    {
        vSemaphoreDelete(g_spi_channel_data[channel].transfer_done_sem);
        g_spi_channel_data[channel].transfer_done_sem = NULL;
    }

    g_spi_channel_data[channel].initialized = false;
    g_spi_channel_data[channel].spi_handle = NULL;
    g_spi_channel_data[channel].callback = NULL;
    LOGI(TAG, "SPI channel %d de-initialized successfully.", channel);
    return HAL_SPI_STATUS_OK;
}

/**
 * @brief Helper function to execute an SPI transaction.
 * @param channel The SPI channel.
 * @param tx_buffer Pointer to transmit buffer.
 * @param rx_buffer Pointer to receive buffer.
 * @param length Length of data in bytes.
 * @param timeout_ms Timeout in milliseconds.
 * @param async True for asynchronous, false for blocking.
 * @return HAL_SPI_Status_t
 */
static HAL_SPI_Status_t spi_execute_transaction(HAL_SPI_Channel_t channel,
                                                const uint8_t *tx_buffer,
                                                uint8_t *rx_buffer,
                                                uint32_t length,
                                                uint32_t timeout_ms,
                                                bool async)
{
    if (channel >= HAL_SPI_CHANNEL_MAX || !g_spi_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "SPI channel %d not initialized or invalid.", channel);
        return HAL_SPI_STATUS_NOT_INITIALIZED;
    }
    if (length == 0 || (tx_buffer == NULL && rx_buffer == NULL))
    {
        ESP_LOGE(TAG, "Invalid parameters: length 0 or both buffers NULL for channel %d.", channel);
        return HAL_SPI_STATUS_INVALID_PARAM;
    }

    spi_transaction_t t = {
        .length = length * 8, // Length in bits
        .tx_buffer = tx_buffer,
        .rx_buffer = rx_buffer,
        .user = (void *)channel, // Pass channel ID to the callback
    };

    esp_err_t ret;
    if (async)
    {
        // Clear semaphore before starting new async transfer
        xSemaphoreTake(g_spi_channel_data[channel].transfer_done_sem, 0);
        g_spi_channel_data[channel].async_status = HAL_SPI_STATUS_BUSY;                          // Mark as busy
        ret = spi_device_queue_trans(g_spi_channel_data[channel].spi_handle, &t, portMAX_DELAY); // Queue transaction
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to queue SPI transaction for channel %d: %s", channel, esp_err_to_name(ret));
            g_spi_channel_data[channel].async_status = HAL_SPI_STATUS_ERROR;
            return HAL_SPI_STATUS_ERROR;
        }
        // For async, we return immediately. The callback will handle completion.
        return HAL_SPI_STATUS_OK;
    }
    else
    {
        ret = spi_device_transmit(g_spi_channel_data[channel].spi_handle, &t);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to transmit/receive SPI data for channel %d: %s", channel, esp_err_to_name(ret));
            return HAL_SPI_STATUS_ERROR;
        }
        return HAL_SPI_STATUS_OK;
    }
}

HAL_SPI_Status_t HAL_SPI_Master_TransmitReceive(HAL_SPI_Channel_t channel,
                                                const uint8_t *tx_buffer,
                                                uint8_t *rx_buffer,
                                                uint32_t length,
                                                uint32_t timeout_ms)
{
    return spi_execute_transaction(channel, tx_buffer, rx_buffer, length, timeout_ms, false);
}

HAL_SPI_Status_t HAL_SPI_Master_Transmit(HAL_SPI_Channel_t channel,
                                         const uint8_t *tx_buffer,
                                         uint32_t length,
                                         uint32_t timeout_ms)
{
    return spi_execute_transaction(channel, tx_buffer, NULL, length, timeout_ms, false);
}

HAL_SPI_Status_t HAL_SPI_Master_Receive(HAL_SPI_Channel_t channel,
                                        uint8_t *rx_buffer,
                                        uint32_t length,
                                        uint32_t timeout_ms)
{
    // For receive-only, ESP-IDF will send dummy bytes automatically.
    return spi_execute_transaction(channel, NULL, rx_buffer, length, timeout_ms, false);
}

HAL_SPI_Status_t HAL_SPI_Master_TransmitReceive_IT(HAL_SPI_Channel_t channel,
                                                   const uint8_t *tx_buffer,
                                                   uint8_t *rx_buffer,
                                                   uint32_t length,
                                                   HAL_SPI_TxRxCpltCallback_t callback)
{
    if (channel >= HAL_SPI_CHANNEL_MAX || !g_spi_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "SPI channel %d not initialized or invalid.", channel);
        return HAL_SPI_STATUS_NOT_INITIALIZED;
    }

    g_spi_channel_data[channel].callback = callback;
    return spi_execute_transaction(channel, tx_buffer, rx_buffer, length, 0, true); // Timeout not used for IT
}

HAL_SPI_Status_t HAL_SPI_Master_Transmit_IT(HAL_SPI_Channel_t channel,
                                            const uint8_t *tx_buffer,
                                            uint32_t length,
                                            HAL_SPI_TxRxCpltCallback_t callback)
{
    if (channel >= HAL_SPI_CHANNEL_MAX || !g_spi_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "SPI channel %d not initialized or invalid.", channel);
        return HAL_SPI_STATUS_NOT_INITIALIZED;
    }

    g_spi_channel_data[channel].callback = callback;
    return spi_execute_transaction(channel, tx_buffer, NULL, length, 0, true);
}

HAL_SPI_Status_t HAL_SPI_Master_Receive_IT(HAL_SPI_Channel_t channel,
                                           uint8_t *rx_buffer,
                                           uint32_t length,
                                           HAL_SPI_TxRxCpltCallback_t callback)
{
    if (channel >= HAL_SPI_CHANNEL_MAX || !g_spi_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "SPI channel %d not initialized or invalid.", channel);
        return HAL_SPI_STATUS_NOT_INITIALIZED;
    }

    g_spi_channel_data[channel].callback = callback;
    return spi_execute_transaction(channel, NULL, rx_buffer, length, 0, true);
}

HAL_SPI_Status_t HAL_SPI_GetStatus(HAL_SPI_Channel_t channel)
{
    if (channel >= HAL_SPI_CHANNEL_MAX || !g_spi_channel_data[channel].initialized)
    {
        return HAL_SPI_STATUS_NOT_INITIALIZED;
    }
    // For blocking transfers, status is implicitly OK upon return, or an error code.
    // For IT transfers, we can return the stored async_status.
    return g_spi_channel_data[channel].async_status;
}

HAL_SPI_Status_t HAL_SPI_SetChipSelect(HAL_SPI_Channel_t channel, uint8_t cs_pin, bool state)
{
    // This function assumes CS is managed as a regular GPIO by higher layers
    // or through a pre-transfer callback if integrated with ESP-IDF's driver.
    // For this generic HAL, we'll provide a direct GPIO control.
    // In a real application, you might integrate this with spi_device_interface_config_t's .spics_io_num
    // or a pre_cb/post_cb for more integrated CS handling.

    if (cs_pin == -1)
    {
        ESP_LOGW(TAG, "Attempted to set CS for channel %d with invalid pin (-1).", channel);
        return HAL_SPI_STATUS_INVALID_PARAM;
    }

    gpio_set_level((gpio_num_t)cs_pin, state ? 0 : 1); // Assuming active low CS
    return HAL_SPI_STATUS_OK;
}
