/**
 * @file hal_spi.h
 * @brief Hardware Abstraction Layer for SPI (Serial Peripheral Interface) - Interface.
 *
 * This module provides a generic interface for interacting with SPI peripherals.
 * It abstracts the underlying hardware details, allowing higher-level modules
 * to perform SPI communication (e.g., master mode, data transfer) without
 * direct hardware register manipulation.
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for available SPI channels/peripherals.
 *
 * Define the specific SPI hardware instances available on the target platform.
 * Add more channels as needed for your ESP32 configuration.
 */
typedef enum
{
    HAL_SPI_CHANNEL_0,  /**< SPI peripheral 0 (e.g., VSPI on ESP32) */
    HAL_SPI_CHANNEL_1,  /**< SPI peripheral 1 (e.g., HSPI on ESP32) */
    // Add more SPI channels if your hardware supports them
    HAL_SPI_CHANNEL_MAX /**< Total number of defined SPI channels */
} HAL_SPI_Channel_t;

/**
 * @brief Enumeration for SPI operating modes.
 *
 * Defines the clock polarity (CPOL) and clock phase (CPHA) settings.
 * CPOL=0: Clock is low when idle.
 * CPOL=1: Clock is high when idle.
 * CPHA=0: Data is sampled on the first clock edge.
 * CPHA=1: Data is sampled on the second clock edge.
 */
typedef enum
{
    HAL_SPI_MODE_0 = 0, /**< CPOL=0, CPHA=0 (SDA sampled on rising edge) */
    HAL_SPI_MODE_1 = 1, /**< CPOL=0, CPHA=1 (SDA sampled on falling edge) */
    HAL_SPI_MODE_2 = 2, /**< CPOL=1, CPHA=0 (SDA sampled on falling edge) */
    HAL_SPI_MODE_3 = 3  /**< CPOL=1, CPHA=1 (SDA sampled on rising edge) */
} HAL_SPI_Mode_t;

/**
 * @brief Enumeration for SPI data bit width.
 *
 * Defines the number of bits per data frame.
 */
typedef enum
{
    HAL_SPI_DATA_BITS_8 = 8,    /**< 8-bit data frame */
    HAL_SPI_DATA_BITS_16 = 16,  /**< 16-bit data frame */
    HAL_SPI_DATA_BITS_32 = 32   /**< 32-bit data frame */
    // Add other common data bit widths if supported by hardware
} HAL_SPI_DataBits_t;

/**
 * @brief Enumeration for SPI status codes.
 */
typedef enum
{
    HAL_SPI_STATUS_OK = 0,          /**< Operation successful */
    HAL_SPI_STATUS_ERROR,           /**< General error */
    HAL_SPI_STATUS_BUSY,            /**< SPI peripheral is currently busy */
    HAL_SPI_STATUS_TIMEOUT,         /**< Operation timed out */
    HAL_SPI_STATUS_INVALID_CHANNEL, /**< Invalid SPI channel specified */
    HAL_SPI_STATUS_INVALID_PARAM,   /**< Invalid parameter provided */
    HAL_SPI_STATUS_NOT_INITIALIZED  /**< SPI channel not initialized */
} HAL_SPI_Status_t;

/**
 * @brief Type definition for SPI transfer completion callback function.
 *
 * This callback is invoked when an asynchronous SPI transfer completes.
 *
 * @param channel The SPI channel on which the transfer completed.
 * @param status The status of the completed transfer (HAL_SPI_STATUS_OK on success).
 */
typedef void (*HAL_SPI_TxRxCpltCallback_t)(HAL_SPI_Channel_t channel, HAL_SPI_Status_t status);

/**
 * @brief Initializes a specific SPI channel with the given configuration.
 *
 * This function sets up the SPI peripheral, including baud rate, mode,
 * and data format. It typically reads configuration from `hal_spi_cfg.c`.
 *
 * @param channel The SPI channel to initialize.
 * @return HAL_SPI_STATUS_OK if initialization is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Init(HAL_SPI_Channel_t channel);

/**
 * @brief De-initializes a specific SPI channel.
 *
 * This function disables the SPI peripheral and releases any associated resources.
 *
 * @param channel The SPI channel to de-initialize.
 * @return HAL_SPI_STATUS_OK if de-initialization is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_DeInit(HAL_SPI_Channel_t channel);

/**
 * @brief Performs a blocking SPI master transmit/receive operation.
 *
 * This function transmits data from `tx_buffer` and simultaneously receives data
 * into `rx_buffer`. The operation blocks until completion.
 *
 * @param channel The SPI channel to use.
 * @param tx_buffer Pointer to the data to transmit (can be NULL if only receiving).
 * @param rx_buffer Pointer to the buffer to store received data (can be NULL if only transmitting).
 * @param length The number of bytes/words to transfer.
 * @param timeout_ms Timeout in milliseconds for the operation. Use 0 for no timeout.
 * @return HAL_SPI_STATUS_OK if transfer is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_TransmitReceive(HAL_SPI_Channel_t channel,
                                                const uint8_t *tx_buffer,
                                                uint8_t *rx_buffer,
                                                uint32_t length,
                                                uint32_t timeout_ms);

/**
 * @brief Performs a blocking SPI master transmit operation.
 *
 * This function transmits data from `tx_buffer`. The operation blocks until completion.
 *
 * @param channel The SPI channel to use.
 * @param tx_buffer Pointer to the data to transmit.
 * @param length The number of bytes/words to transmit.
 * @param timeout_ms Timeout in milliseconds for the operation. Use 0 for no timeout.
 * @return HAL_SPI_STATUS_OK if transfer is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_Transmit(HAL_SPI_Channel_t channel,
                                         const uint8_t *tx_buffer,
                                         uint32_t length,
                                         uint32_t timeout_ms);

/**
 * @brief Performs a blocking SPI master receive operation.
 *
 * This function receives data into `rx_buffer`. The operation blocks until completion.
 * During receive-only operations, the master typically sends dummy bytes (e.g., 0x00).
 *
 * @param channel The SPI channel to use.
 * @param rx_buffer Pointer to the buffer to store received data.
 * @param length The number of bytes/words to receive.
 * @param timeout_ms Timeout in milliseconds for the operation. Use 0 for no timeout.
 * @return HAL_SPI_STATUS_OK if transfer is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_Receive(HAL_SPI_Channel_t channel,
                                        uint8_t *rx_buffer,
                                        uint32_t length,
                                        uint32_t timeout_ms);

/**
 * @brief Performs an asynchronous SPI master transmit/receive operation.
 *
 * This function initiates a non-blocking transfer. The `callback` function
 * will be invoked upon completion or error.
 *
 * @param channel The SPI channel to use.
 * @param tx_buffer Pointer to the data to transmit (can be NULL if only receiving).
 * @param rx_buffer Pointer to the buffer to store received data (can be NULL if only transmitting).
 * @param length The number of bytes/words to transfer.
 * @param callback Optional: Pointer to a callback function to be called upon completion.
 * Pass NULL if no callback is desired (e.g., for polling status).
 * @return HAL_SPI_STATUS_OK if the operation is successfully initiated, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_TransmitReceive_IT(HAL_SPI_Channel_t channel,
                                                   const uint8_t *tx_buffer,
                                                   uint8_t *rx_buffer,
                                                   uint32_t length,
                                                   HAL_SPI_TxRxCpltCallback_t callback);

/**
 * @brief Performs an asynchronous SPI master transmit operation.
 *
 * This function initiates a non-blocking transmit. The `callback` function
 * will be invoked upon completion or error.
 *
 * @param channel The SPI channel to use.
 * @param tx_buffer Pointer to the data to transmit.
 * @param length The number of bytes/words to transmit.
 * @param callback Optional: Pointer to a callback function to be called upon completion.
 * @return HAL_SPI_STATUS_OK if the operation is successfully initiated, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_Transmit_IT(HAL_SPI_Channel_t channel,
                                            const uint8_t *tx_buffer,
                                            uint32_t length,
                                            HAL_SPI_TxRxCpltCallback_t callback);

/**
 * @brief Performs an asynchronous SPI master receive operation.
 *
 * This function initiates a non-blocking receive. The `callback` function
 * will be invoked upon completion or error.
 *
 * @param channel The SPI channel to use.
 * @param rx_buffer Pointer to the buffer to store received data.
 * @param length The number of bytes/words to receive.
 * @param callback Optional: Pointer to a callback function to be called upon completion.
 * @return HAL_SPI_STATUS_OK if the operation is successfully initiated, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_Master_Receive_IT(HAL_SPI_Channel_t channel,
                                           uint8_t *rx_buffer,
                                           uint32_t length,
                                           HAL_SPI_TxRxCpltCallback_t callback);

/**
 * @brief Gets the current status of an SPI channel.
 *
 * @param channel The SPI channel to query.
 * @return The current status of the SPI channel (e.g., HAL_SPI_STATUS_BUSY, HAL_SPI_STATUS_OK).
 */
HAL_SPI_Status_t HAL_SPI_GetStatus(HAL_SPI_Channel_t channel);

/**
 * @brief Sets the Chip Select (CS) line for a specific SPI device.
 *
 * This function is typically used by higher layers to control the CS line
 * when multiple devices share the same SPI bus. The HAL itself might manage
 * CS for single-device scenarios or if it's explicitly configured to do so.
 *
 * @param channel The SPI channel associated with the device.
 * @param cs_pin The GPIO pin number connected to the device's CS line.
 * @param state True to assert (active low), false to de-assert (inactive high).
 * @return HAL_SPI_STATUS_OK if the operation is successful, otherwise an error code.
 */
HAL_SPI_Status_t HAL_SPI_SetChipSelect(HAL_SPI_Channel_t channel, uint8_t cs_pin, bool state);

#endif /* HAL_SPI_H */
