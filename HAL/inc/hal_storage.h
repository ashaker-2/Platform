/**
 * @file hal_storage.h
 * @brief Hardware Abstraction Layer for Non-Volatile Storage.
 *
 * This module provides an abstract interface for interacting with the
 * underlying non-volatile memory hardware, specifically distinguishing
 * between Flash memory and EEPROM (or emulated EEPROM).
 *
 * This naming convention avoids conflicts with a higher-level Non-Volatile
 * Memory (NvM) Service Layer component, which would manage application
 * data persistence using these HAL primitives.
 */

#ifndef HAL_STORAGE_H
#define HAL_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enumeration for HAL Storage return codes.
 */
typedef enum
{
    HAL_STORAGE_OK = 0,         /**< Operation successful */
    HAL_STORAGE_ERROR,          /**< General error */
    HAL_STORAGE_INVALID_PARAM,  /**< Invalid parameter provided */
    HAL_STORAGE_WRITE_FAIL,     /**< Write operation failed */
    HAL_STORAGE_ERASE_FAIL,     /**< Erase operation failed */
    HAL_STORAGE_BUSY,           /**< Storage is busy with another operation */
    HAL_STORAGE_NOT_READY       /**< Storage is not initialized or ready */
} HAL_Storage_Status_t;

/* --- Flash Memory Operations --- */
/**
 * @brief Reads data from Flash memory.
 * @param address The starting address in Flash memory to read from.
 * @param data_buffer Pointer to the buffer where read data will be stored.
 * @param length The number of bytes to read.
 * @return HAL_STORAGE_OK if successful, otherwise an error code.
 */
HAL_Storage_Status_t HAL_Storage_Flash_Read(uint32_t address, uint8_t *data_buffer, uint32_t length);

/**
 * @brief Writes data to Flash memory.
 * @note Flash memory typically requires erasing before writing. This function
 * might handle erasing internally for the affected pages/sectors, or
 * it might assume the area is already erased. Refer to specific
 * implementation details.
 * @param address The starting address in Flash memory to write to.
 * @param data_buffer Pointer to the data to write.
 * @param length The number of bytes to write.
 * @return HAL_STORAGE_OK if successful, otherwise an error code.
 */
HAL_Storage_Status_t HAL_Storage_Flash_Write(uint32_t address, const uint8_t *data_buffer, uint32_t length);

/**
 * @brief Erases a sector in Flash memory.
 * @param sector_address The starting address of the sector to erase.
 * @return HAL_STORAGE_OK if successful, otherwise an error code.
 */
HAL_Storage_Status_t HAL_Storage_Flash_EraseSector(uint32_t sector_address);

/**
 * @brief Gets the total size of the Flash memory in bytes.
 * @return The size of Flash memory in bytes.
 */
uint32_t HAL_Storage_Flash_GetSize(void);

/**
 * @brief Gets the size of a single Flash sector in bytes.
 * @return The size of a Flash sector in bytes.
 */
uint32_t HAL_Storage_Flash_GetSectorSize(void);


/* --- EEPROM (or Emulated NVM) Operations --- */
/**
 * @brief Reads data from EEPROM (or emulated NVM).
 * @param address The starting address in EEPROM to read from.
 * @param data_buffer Pointer to the buffer where read data will be stored.
 * @param length The number of bytes to read.
 * @return HAL_STORAGE_OK if successful, otherwise an error code.
 */
HAL_Storage_Status_t HAL_Storage_EEPROM_Read(uint32_t address, uint8_t *data_buffer, uint32_t length);

/**
 * @brief Writes data to EEPROM (or emulated NVM).
 * @param address The starting address in EEPROM to write to.
 * @param data_buffer Pointer to the data to write.
 * @param length The number of bytes to write.
 * @return HAL_STORAGE_OK if successful, otherwise an error code.
 */
HAL_Storage_Status_t HAL_Storage_EEPROM_Write(uint32_t address, const uint8_t *data_buffer, uint32_t length);

/**
 * @brief Gets the total size of the EEPROM (or emulated NVM) in bytes.
 * @return The size of EEPROM in bytes.
 */
uint32_t HAL_Storage_EEPROM_GetSize(void);

#endif /* HAL_STORAGE_H */
