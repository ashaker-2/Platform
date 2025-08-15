/**
 * @file nvm.h
 * @brief Non-Volatile Memory (NVM) Service - Interface Header.
 *
 * This header defines the public API for the NVM service, providing
 * functions to initialize, read, write, and commit configuration data
 * to non-volatile storage (e.g., EEPROM, Flash memory). It abstracts
 * the underlying storage mechanism and provides a block-based access
 * model for application-specific data.
 */

#ifndef NVM_H
#define NVM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // For uint16_t

/**
 * @brief Enumeration for NVM Service status codes.
 */
typedef enum
{
    NVM_STATUS_OK = 0,                  /**< Operation successful. */
    NVM_STATUS_ERROR,                   /**< Generic error. */
    NVM_STATUS_INVALID_PARAM,           /**< Invalid parameter provided. */
    NVM_STATUS_NOT_INITIALIZED,         /**< NVM service not initialized. */
    NVM_STATUS_ALREADY_INITIALIZED,     /**< NVM service already initialized. */
    NVM_STATUS_BUSY,                    /**< NVM operation is busy. */
    NVM_STATUS_TIMEOUT,                 /**< Timeout during NVM access. */
    NVM_STATUS_CRC_ERROR,               /**< Data integrity check (CRC) failed. */
    NVM_STATUS_READ_ERROR,              /**< Error during NVM read operation. */
    NVM_STATUS_WRITE_ERROR,             /**< Error during NVM write operation. */
    NVM_STATUS_ERASE_ERROR,             /**< Error during NVM erase operation. */
    NVM_STATUS_INVALID_BLOCK_ID,        /**< Provided block ID is not configured. */
    NVM_STATUS_DATA_TOO_LARGE,          /**< Data size exceeds block capacity. */
    NVM_STATUS_NO_CHANGES,              /**< No changes to commit. */
    NVM_STATUS_MAX                      /**< Maximum number of status codes. */
} NVM_Status_t;

/**
 * @brief Initializes the NVM service.
 *
 * This function prepares the NVM module for operation. It typically involves
 * initializing the underlying hardware abstraction layer, checking the NVM
 * integrity, and loading initial data into RAM buffers if necessary.
 *
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 */
NVM_Status_t NVM_Init(void);

/**
 * @brief De-initializes the NVM service.
 *
 * This function releases resources used by the NVM module. Any pending
 * writes might be committed before de-initialization.
 *
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 */
NVM_Status_t NVM_DeInit(void);

/**
 * @brief Reads data from a specified NVM block into a buffer.
 *
 * The NVM service maintains internal RAM buffers for each configured block.
 * This function reads data from the RAM buffer. If `NVM_Init` performs
 * lazy loading, the data might be read from physical NVM upon the first access.
 *
 * @param block_id The unique identifier of the NVM block to read.
 * @param buffer Pointer to the destination buffer where data will be copied.
 * @param buffer_len The size of the destination buffer in bytes.
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 * NVM_STATUS_DATA_TOO_LARGE if `buffer_len` is less than the block's configured size.
 */
NVM_Status_t NVM_Read(uint32_t block_id, void *buffer, uint16_t buffer_len);

/**
 * @brief Writes data to a specified NVM block's RAM buffer.
 *
 * This function writes data to the internal RAM buffer associated with the
 * NVM block. The changes are not immediately committed to physical NVM.
 * A subsequent call to `NVM_Commit` is required to save the changes permanently.
 *
 * @param block_id The unique identifier of the NVM block to write.
 * @param data Pointer to the source data to be written.
 * @param data_len The size of the source data in bytes.
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 * NVM_STATUS_DATA_TOO_LARGE if `data_len` exceeds the block's configured size.
 */
NVM_Status_t NVM_Write(uint32_t block_id, const void *data, uint16_t data_len);

/**
 * @brief Commits all pending changes from RAM buffers to physical NVM.
 *
 * This function iterates through all NVM blocks that have been marked as 'dirty'
 * (i.e., modified by `NVM_Write`) and writes their updated content to the
 * physical non-volatile memory. This operation can be time-consuming depending
 * on the underlying NVM technology (e.g., Flash erase/write cycles).
 *
 * @return NVM_STATUS_OK if all changes were successfully committed,
 * an error code if any block failed to commit.
 */
NVM_Status_t NVM_Commit(void);

/**
 * @brief Formats the entire NVM area, resetting all blocks to their default values.
 *
 * This function erases the entire NVM storage area managed by the service
 * and then writes the default values for all configured NVM blocks.
 * This is typically used for factory reset or initial provisioning.
 *
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 */
NVM_Status_t NVM_Format(void);

#endif /* NVM_H */
