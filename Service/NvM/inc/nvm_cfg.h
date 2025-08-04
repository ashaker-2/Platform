/**
 * @file nvm_cfg.h
 * @brief Non-Volatile Memory (NVM) Service - Configuration Header.
 *
 * This header defines the compile-time configuration parameters for the NVM service.
 * It specifies the structure of NVM blocks, their unique identifiers, sizes,
 * and pointers to their default values. These configurations are typically
 * defined in `nvm_cfg.c`.
 */

#ifndef NVM_CFG_H
#define NVM_CFG_H

#include <stdint.h>
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique identifiers of NVM data blocks.
 *
 * Each entry here represents a distinct logical block of data
 * that the application wishes to store in non-volatile memory.
 * The values should be unique and sequential starting from 0.
 */
typedef enum
{
    NVM_BLOCK_ID_SYSTEM_SETTINGS = 0,   /**< Block for general system configuration. */
    NVM_BLOCK_ID_CALIBRATION_DATA,      /**< Block for sensor calibration data. */
    NVM_BLOCK_ID_DEVICE_INFO,           /**< Block for device specific information (e.g., serial number). */
    NVM_BLOCK_ID_LOG_POINTER,           /**< Block to store the current log write pointer. */
    // Add more NVM block IDs as needed for your application
    NVM_BLOCK_ID_MAX                    /**< Total number of configured NVM blocks. */
} NVM_BlockId_t;

/**
 * @brief Structure to define the configuration for a single NVM data block.
 */
typedef struct
{
    uint32_t id;                        /**< Unique identifier for this NVM block (from NVM_BlockId_t). */
    size_t size;                        /**< Size of the data block in bytes. */
    const void *default_value_ptr;      /**< Pointer to the default values for this block.
                                             This data will be written if the NVM is formatted or corrupted. */
} NVM_BlockConfig_t;

/**
 * @brief External declaration for the global array of NVM block configurations.
 *
 * This array is defined in `nvm_cfg.c` and provides the static
 * configuration for all NVM blocks managed by the service.
 */
extern const NVM_BlockConfig_t g_nvm_block_configs[NVM_BLOCK_ID_MAX];

/**
 * @brief Define the base address in the physical NVM where the NVM service starts storing data.
 *
 * This address should be carefully chosen to avoid conflicts with bootloaders,
 * firmware images, or other persistent data areas. It is typically an offset
 * within the Flash memory or the starting address of an EEPROM.
 *
 * NOTE: This is a placeholder. In a real system, this would be specific
 * to the memory map of your chosen hardware (e.g., ESP32 partition table).
 */
#define NVM_PHYSICAL_BASE_ADDRESS       0x100000 // Example: Start at 1MB offset in Flash

/**
 * @brief Define the sector size of the underlying NVM (e.g., Flash sector size).
 *
 * This is crucial for erase operations, as Flash memory typically must be erased
 * in fixed-size sectors. All NVM blocks should align with or fit within these sectors.
 *
 * NOTE: This is a placeholder. For ESP32, common sector size is 4KB.
 */
#define NVM_FLASH_SECTOR_SIZE           0x1000 // 4KB

/**
 * @brief Define the maximum size of the internal RAM buffer used for NVM operations.
 *
 * This buffer is used for temporary storage during read/write operations
 * and should be large enough to hold the largest NVM block.
 */
#define NVM_MAX_BLOCK_SIZE_BYTES        256 // Example: Max 256 bytes for any single NVM block

/**
 * @brief Define the size of the CRC checksum used for data integrity.
 */
#define NVM_CRC_SIZE_BYTES              2 // Using CRC16

#endif /* NVM_CFG_H */
