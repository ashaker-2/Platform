/**
 * @file hal_spi_cfg.h
 * @brief Hardware Abstraction Layer for SPI (Serial Peripheral Interface) - Configuration Header.
 *
 * This header defines the configuration structure for SPI channels.
 * It is used by `hal_spi_cfg.c` to provide specific settings for each
 * SPI peripheral on the target hardware.
 */

#ifndef HAL_SPI_CFG_H
#define HAL_SPI_CFG_H

#include <stdint.h>
#include <stdbool.h>
#include "hal_spi.h" // Include the main SPI HAL interface for enums and types

/**
 * @brief Structure to hold compile-time configuration for a single SPI channel.
 *
 * This structure defines all the necessary parameters to initialize and
 * operate a specific SPI peripheral.
 */
typedef struct
{
    HAL_SPI_Channel_t channel_id;       /**< Unique identifier for the SPI channel. */
    uint32_t baud_rate_hz;              /**< SPI clock frequency in Hz (e.g., 10,000,000 for 10 MHz). */
    HAL_SPI_Mode_t spi_mode;            /**< SPI mode (CPOL/CPHA combination). */
    HAL_SPI_DataBits_t data_bits;       /**< Number of data bits per transfer (e.g., 8, 16, 32). */
    bool master_mode;                   /**< True if operating as master, false for slave. */
    // Pin assignments (example for ESP32, adjust as per actual HAL implementation)
    int8_t mosi_pin;                    /**< GPIO pin for Master Out Slave In (MOSI). Use -1 if not used/configured by HAL. */
    int8_t miso_pin;                    /**< GPIO pin for Master In Slave Out (MISO). Use -1 if not used/configured by HAL. */
    int8_t sclk_pin;                    /**< GPIO pin for Serial Clock (SCLK). Use -1 if not used/configured by HAL. */
    // Note: Chip Select (CS) pins are typically managed by the application layer
    // if multiple devices share the bus, or can be specified here if one-to-one.
    // For simplicity, CS is handled by HAL_SPI_SetChipSelect or higher layers.

    HAL_SPI_TxRxCpltCallback_t txrx_cplt_callback; /**< Callback for asynchronous transfer completion. */
    // Add any other specific hardware-related configurations here, e.g.,
    // bool use_dma; // Whether to use DMA for transfers
    // uint32_t dma_channel; // Specific DMA channel if applicable

} HAL_SPI_ChannelConfig_t;

/**
 * @brief External declaration for the array of SPI channel configurations.
 *
 * This array is defined in `hal_spi_cfg.c` and holds the static configuration
 * for each SPI channel.
 */
extern const HAL_SPI_ChannelConfig_t g_hal_spi_channel_configs[HAL_SPI_CHANNEL_MAX];

#endif /* HAL_SPI_CFG_H */
