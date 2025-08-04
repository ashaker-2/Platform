/**
 * @file hal_spi_cfg.c
 * @brief Hardware Abstraction Layer for SPI (Serial Peripheral Interface) - Configuration Implementation.
 *
 * This file defines the static configuration parameters for each SPI channel
 * available on the target hardware. These configurations are used by the
 * `hal_spi.c` implementation to initialize and operate the SPI peripherals.
 *
 * IMPORTANT: Adjust the pin assignments, baud rates, and modes according to
 * your specific ESP32 hardware connections and requirements.
 */

#include "hal_spi_cfg.h"
#include "hal_spi.h" // For HAL_SPI_Channel_t and other enums
#include "driver/spi_master.h" // ESP-IDF SPI driver
#include "driver/gpio.h"       // ESP-IDF GPIO driver

// Forward declarations for any callbacks if they were to be defined here.
// For now, callbacks are expected to be passed during IT transfers or
// managed by higher layers.

/**
 * @brief Array of SPI channel configurations.
 *
 * This array holds the compile-time configuration for each SPI channel.
 * The index of the array corresponds to the HAL_SPI_Channel_t enum value.
 */
const HAL_SPI_ChannelConfig_t g_hal_spi_channel_configs[HAL_SPI_CHANNEL_MAX] = {
    [HAL_SPI_CHANNEL_0] = {
        .channel_id = HAL_SPI_CHANNEL_0,
        .baud_rate_hz = 10 * 1000 * 1000, // Example: 10 MHz
        .spi_mode = HAL_SPI_MODE_0,       // Example: CPOL=0, CPHA=0
        .data_bits = HAL_SPI_DATA_BITS_8, // Example: 8-bit data
        .master_mode = true,              // Operating as master
        .mosi_pin = GPIO_NUM_23,          // Example: GPIO23 for MOSI
        .miso_pin = GPIO_NUM_19,          // Example: GPIO19 for MISO
        .sclk_pin = GPIO_NUM_18,          // Example: GPIO18 for SCLK
        .txrx_cplt_callback = NULL,       // No default callback, set during IT transfer
    },
    [HAL_SPI_CHANNEL_1] = {
        .channel_id = HAL_SPI_CHANNEL_1,
        .baud_rate_hz = 5 * 1000 * 1000,  // Example: 5 MHz
        .spi_mode = HAL_SPI_MODE_3,       // Example: CPOL=1, CPHA=1
        .data_bits = HAL_SPI_DATA_BITS_8, // Example: 8-bit data
        .master_mode = true,              // Operating as master
        .mosi_pin = GPIO_NUM_27,          // Example: GPIO27 for MOSI
        .miso_pin = GPIO_NUM_26,          // Example: GPIO26 for MISO
        .sclk_pin = GPIO_NUM_25,          // Example: GPIO25 for SCLK
        .txrx_cplt_callback = NULL,       // No default callback, set during IT transfer
    },
    // Add configurations for more SPI channels if defined in hal_spi.h
    // Example:
    /*
    [HAL_SPI_CHANNEL_2] = {
        .channel_id = HAL_SPI_CHANNEL_2,
        .baud_rate_hz = 20 * 1000 * 1000, // Example: 20 MHz
        .spi_mode = HAL_SPI_MODE_1,
        .data_bits = HAL_SPI_DATA_BITS_8,
        .master_mode = true,
        .mosi_pin = GPIO_NUM_13,
        .miso_pin = GPIO_NUM_12,
        .sclk_pin = GPIO_NUM_14,
        .txrx_cplt_callback = NULL,
    },
    */
};

