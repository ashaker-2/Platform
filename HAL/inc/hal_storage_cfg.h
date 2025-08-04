/**
 * @file hal_storage_cfg.h
 * @brief Configuration header for the HAL_Storage module.
 *
 * This file defines configurable parameters for the HAL_Storage module,
 * such as memory sizes and sector sizes, which are specific to the
 * target hardware platform (e.g., ESP32).
 */

#ifndef HAL_STORAGE_CFG_H
#define HAL_STORAGE_CFG_H

#include <stdint.h>

// --- Flash Memory Configuration ---
// These values would typically come from the ESP32's flash partition table
// or specific hardware specifications.

/**
 * @brief Total size of the Flash memory available for application use in bytes.
 * (Example: 4MB Flash, part of it used by bootloader, OTA partitions, etc.)
 * Adjust this based on your ESP32's actual usable Flash size.
 */
#define HAL_STORAGE_FLASH_TOTAL_SIZE    (4 * 1024 * 1024) // Example: 4 MB

/**
 * @brief Size of a single Flash memory sector in bytes.
 * (Commonly 4KB on ESP32, but verify your specific chip/configuration).
 */
#define HAL_STORAGE_FLASH_SECTOR_SIZE   (4 * 1024)        // Example: 4 KB

// --- EEPROM (or Emulated NVM) Configuration ---
// On ESP32, this often refers to a partition managed by the NVS (Non-Volatile Storage)
// component, which provides key-value pair storage and can be used to emulate EEPROM.

/**
 * @brief Total size of the EEPROM (or NVS partition used for emulated NVM) in bytes.
 * Adjust this based on your ESP32's NVS partition size.
 */
#define HAL_STORAGE_EEPROM_TOTAL_SIZE   (64 * 1024)       // Example: 64 KB for NVS partition

#endif /* HAL_STORAGE_CFG_H */
