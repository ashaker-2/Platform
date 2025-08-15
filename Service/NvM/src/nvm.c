/**
 * @file nvm.c
 * @brief Non-Volatile Memory (NVM) Service - Implementation.
 *
 * This module provides the concrete implementation for the NVM service interface
 * defined in `nvm.h`. It manages the storage and retrieval of application
 * configuration data in non-volatile memory (e.g., Flash).
 *
 * It includes features like:
 * - Block-based access to NVM data.
 * - In-RAM caching of NVM blocks for faster access.
 * - "Dirty" flag mechanism to track modified blocks before committing to NVM.
 * - CRC16 checksum for data integrity validation.
 * - Integration with a hypothetical Hardware Abstraction Layer (HAL) for NVM access.
 * - Thread-safety using a FreeRTOS mutex.
 *
 * NOTE: This implementation assumes the existence of a `hal_nvm.h` and
 * corresponding `HAL_NVM_Read`, `HAL_NVM_Write`, `HAL_NVM_Erase` functions
 * which interact directly with the physical non-volatile memory (e.g., ESP-IDF Flash API).
 * These HAL functions are placeholders and need to be provided by the user.
 */

#include "nvm.h"
#include "nvm_cfg.h" // NVM configuration

// #include "esp_log.h"           // ESP-IDF logging
// #include "freertos/FreeRTOS.h" // For FreeRTOS types
// #include "freertos/semphr.h"   // For mutex

// --- Hypothetical NVM HAL Interface (PLACEHOLDER) ---
// In a real project, these would be defined in a separate hal_nvm.h and hal_nvm.c
// and would interact with the specific hardware (e.g., ESP-IDF NVS, SPI Flash driver).

/**
 * @brief Hypothetical HAL function to read data from physical NVM.
 * @param address The physical memory address to start reading from.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size The number of bytes to read.
 * @return ESP_OK if successful, an error code otherwise.
 */
static esp_err_t HAL_NVM_Read(uint32_t address, void *buffer, uint16_t size)
{
    // Placeholder: Simulate reading from NVM
    // In a real system, this would call actual flash/EEPROM read functions.
    ESP_LOGD("HAL_NVM", "HAL_NVM_Read: addr=0x%08lX, size=%zu", address, size);
    // For simulation, assume success and fill buffer with dummy data or zeros
    // For now, just return OK. Actual data will be handled by the NVM service logic.
    return ESP_OK;
}

/**
 * @brief Hypothetical HAL function to write data to physical NVM.
 * @param address The physical memory address to start writing to.
 * @param data Pointer to the data to write.
 * @param size The number of bytes to write.
 * @return ESP_OK if successful, an error code otherwise.
 */
static esp_err_t HAL_NVM_Write(uint32_t address, const void *data, uint16_t size)
{
    // Placeholder: Simulate writing to NVM
    // In a real system, this would call actual flash/EEPROM write functions.
    ESP_LOGD("HAL_NVM", "HAL_NVM_Write: addr=0x%08lX, size=%zu", address, size);
    // For now, just return OK.
    return ESP_OK;
}

/**
 * @brief Hypothetical HAL function to erase a sector in physical NVM.
 * @param sector_address The physical memory address of the sector to erase.
 * @return ESP_OK if successful, an error code otherwise.
 */
static esp_err_t HAL_NVM_Erase(uint32_t sector_address)
{
    // Placeholder: Simulate erasing a flash sector
    // In a real system, this would call actual flash erase functions.
    ESP_LOGD("HAL_NVM", "HAL_NVM_Erase: sector_addr=0x%08lX", sector_address);
    // For now, just return OK.
    return ESP_OK;
}

// --- End of Hypothetical NVM HAL Interface ---

// Define a tag for ESP-IDF logging
static const char *TAG = "NVM";

/**
 * @brief Structure to hold runtime data for each NVM block.
 */
typedef struct
{
    uint8_t data[NVM_MAX_BLOCK_SIZE_BYTES]; /**< RAM buffer for the NVM block data. */
    bool is_dirty;                          /**< Flag indicating if the RAM buffer has been modified. */
    bool is_loaded;                         /**< Flag indicating if data has been loaded from NVM into RAM. */
} NVM_BlockRuntimeData_t;

// Array to hold runtime data for each NVM block
static NVM_BlockRuntimeData_t g_nvm_runtime_data[NVM_BLOCK_ID_MAX];

static bool g_nvm_initialized = false;       /**< Flag indicating if the NVM service is initialized. */
// static SemaphoreHandle_t g_nvm_mutex = NULL; /**< Mutex to protect NVM operations. */

/**
 * @brief Calculates the CRC16 checksum for a given data buffer.
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return The calculated CRC16 value.
 */
static uint16_t nvm_calculate_crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++)
    {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief Internal helper to get the physical address for a given NVM block ID.
 * @param block_id The ID of the NVM block.
 * @param physical_address Pointer to store the calculated physical address.
 * @return NVM_STATUS_OK if successful, NVM_STATUS_INVALID_BLOCK_ID if ID is out of bounds.
 */
static NVM_Status_t nvm_get_physical_address(uint32_t block_id, uint32_t *physical_address)
{
    if (block_id >= NVM_BLOCK_ID_MAX)
    {
        return NVM_STATUS_INVALID_BLOCK_ID;
    }

    uint32_t current_offset = 0;
    for (uint32_t i = 0; i < block_id; i++)
    {
        // Each block stores its data + CRC
        current_offset += (g_nvm_block_configs[i].size + NVM_CRC_SIZE_BYTES);
    }
    *physical_address = NVM_PHYSICAL_BASE_ADDRESS + current_offset;
    return NVM_STATUS_OK;
}

/**
 * @brief Internal helper to load a specific NVM block from physical memory into RAM.
 * @param block_id The ID of the NVM block to load.
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 */
static NVM_Status_t nvm_load_block_from_nvm(uint32_t block_id)
{
    const NVM_BlockConfig_t *cfg = &g_nvm_block_configs[block_id];
    NVM_BlockRuntimeData_t *runtime_data = &g_nvm_runtime_data[block_id];
    uint32_t physical_addr;
    NVM_Status_t status = nvm_get_physical_address(block_id, &physical_addr);
    if (status != NVM_STATUS_OK)
    {
        return status;
    }

    uint8_t temp_buffer[cfg->size + NVM_CRC_SIZE_BYTES];
    esp_err_t hal_ret = HAL_NVM_Read(physical_addr, temp_buffer, cfg->size + NVM_CRC_SIZE_BYTES);

    if (hal_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read NVM block %lu from physical memory: %s", block_id, esp_err_to_name(hal_ret));
        return NVM_STATUS_READ_ERROR;
    }

    uint16_t stored_crc = (temp_buffer[cfg->size + 1] << 8) | temp_buffer[cfg->size];
    uint16_t calculated_crc = nvm_calculate_crc16(temp_buffer, cfg->size);

    if (stored_crc != calculated_crc)
    {
        ESP_LOGW(TAG, "CRC mismatch for NVM block %lu (Stored: 0x%04X, Calc: 0x%04X). Loading default values.",
                 block_id, stored_crc, calculated_crc);
        // Load default values if CRC fails
        memcpy(runtime_data->data, cfg->default_value_ptr, cfg->size);
        runtime_data->is_dirty = true; // Mark as dirty so it gets written on next commit
        runtime_data->is_loaded = true;
        return NVM_STATUS_CRC_ERROR; // Indicate CRC error, but continue with defaults
    }
    else
    {
        memcpy(runtime_data->data, temp_buffer, cfg->size);
        runtime_data->is_dirty = false;
        runtime_data->is_loaded = true;
        ESP_LOGD(TAG, "NVM block %lu loaded successfully from physical memory.", block_id);
        return NVM_STATUS_OK;
    }
}

/**
 * @brief Internal helper to save a specific NVM block from RAM to physical memory.
 * @param block_id The ID of the NVM block to save.
 * @return NVM_STATUS_OK if successful, an error code otherwise.
 */
static NVM_Status_t nvm_save_block_to_nvm(uint32_t block_id)
{
    const NVM_BlockConfig_t *cfg = &g_nvm_block_configs[block_id];
    NVM_BlockRuntimeData_t *runtime_data = &g_nvm_runtime_data[block_id];
    uint32_t physical_addr;
    NVM_Status_t status = nvm_get_physical_address(block_id, &physical_addr);
    if (status != NVM_STATUS_OK)
    {
        return status;
    }

    uint8_t temp_buffer[cfg->size + NVM_CRC_SIZE_BYTES];
    memcpy(temp_buffer, runtime_data->data, cfg->size);
    uint16_t crc = nvm_calculate_crc16(temp_buffer, cfg->size);
    temp_buffer[cfg->size] = (uint8_t)(crc & 0xFF);
    temp_buffer[cfg->size + 1] = (uint8_t)((crc >> 8) & 0xFF);

    // Determine the sector to erase. This is a simplification.
    // In a real system with wear leveling, this would be more complex.
    uint32_t sector_addr = physical_addr & (~(NVM_FLASH_SECTOR_SIZE - 1));

    // Erase the sector before writing (Flash memory requirement)
    esp_err_t hal_ret = HAL_NVM_Erase(sector_addr);
    if (hal_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to erase NVM sector 0x%08lX for block %lu: %s", sector_addr, block_id, esp_err_to_name(hal_ret));
        return NVM_STATUS_ERASE_ERROR;
    }

    hal_ret = HAL_NVM_Write(physical_addr, temp_buffer, cfg->size + NVM_CRC_SIZE_BYTES);
    if (hal_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write NVM block %lu to physical memory: %s", block_id, esp_err_to_name(hal_ret));
        return NVM_STATUS_WRITE_ERROR;
    }

    runtime_data->is_dirty = false;
    ESP_LOGI(TAG, "NVM block %lu saved successfully to physical memory.", block_id);
    return NVM_STATUS_OK;
}

NVM_Status_t NVM_Init(void)
{
    if (g_nvm_initialized)
    {
        ESP_LOGW(TAG, "NVM service already initialized.");
        return NVM_STATUS_ALREADY_INITIALIZED;
    }

    g_nvm_mutex = xSemaphoreCreateMutex();
    if (g_nvm_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create NVM mutex.");
        return NVM_STATUS_ERROR;
    }

    if (xSemaphoreTake(g_nvm_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to take NVM mutex during init.");
        return NVM_STATUS_BUSY;
    }

    // Initialize runtime data for all blocks
    for (uint32_t i = 0; i < NVM_BLOCK_ID_MAX; i++)
    {
        g_nvm_runtime_data[i].is_dirty = false;
        g_nvm_runtime_data[i].is_loaded = false;

        // Verify block size against max buffer size
        if (g_nvm_block_configs[i].size > NVM_MAX_BLOCK_SIZE_BYTES)
        {
            ESP_LOGE(TAG, "NVM block %lu configured size (%zu) exceeds NVM_MAX_BLOCK_SIZE_BYTES (%u)!",
                     i, g_nvm_block_configs[i].size, NVM_MAX_BLOCK_SIZE_BYTES);
            xSemaphoreGive(g_nvm_mutex);
            vSemaphoreDelete(g_nvm_mutex);
            g_nvm_mutex = NULL;
            return NVM_STATUS_ERROR; // Critical error, cannot proceed
        }

        // Attempt to load each block from NVM. If CRC fails, defaults are loaded.
        NVM_Status_t load_status = nvm_load_block_from_nvm(i);
        if (load_status != NVM_STATUS_OK && load_status != NVM_STATUS_CRC_ERROR)
        {
            ESP_LOGE(TAG, "Failed to load NVM block %lu during initialization. Status: %d", i, load_status);
            // Decide if this is a fatal error or if we can proceed with defaults for this block.
            // For now, we'll mark it as loaded with defaults and continue.
            memcpy(g_nvm_runtime_data[i].data, g_nvm_block_configs[i].default_value_ptr, g_nvm_block_configs[i].size);
            g_nvm_runtime_data[i].is_dirty = true; // Mark as dirty to write defaults on first commit
            g_nvm_runtime_data[i].is_loaded = true;
        }
    }

    g_nvm_initialized = true;
    xSemaphoreGive(g_nvm_mutex);
    ESP_LOGI(TAG, "NVM service initialized successfully.");
    return NVM_STATUS_OK;
}

NVM_Status_t NVM_DeInit(void)
{
    if (!g_nvm_initialized)
    {
        ESP_LOGW(TAG, "NVM service not initialized.");
        return NVM_STATUS_NOT_INITIALIZED;
    }

    NVM_Status_t status = NVM_Commit(); // Attempt to commit any pending changes
    if (status != NVM_STATUS_OK && status != NVM_STATUS_NO_CHANGES)
    {
        ESP_LOGW(TAG, "NVM_DeInit: Failed to commit pending changes during de-initialization. Status: %d", status);
        // Continue de-initialization even if commit failed
    }

    if (g_nvm_mutex != NULL)
    {
        vSemaphoreDelete(g_nvm_mutex);
        g_nvm_mutex = NULL;
    }

    g_nvm_initialized = false;
    ESP_LOGI(TAG, "NVM service de-initialized successfully.");
    return NVM_STATUS_OK;
}

NVM_Status_t NVM_Read(uint32_t block_id, void *buffer, uint16_t buffer_len)
{
    if (!g_nvm_initialized)
    {
        return NVM_STATUS_NOT_INITIALIZED;
    }
    if (block_id >= NVM_BLOCK_ID_MAX || buffer == NULL || buffer_len == 0)
    {
        ESP_LOGE(TAG, "NVM_Read: Invalid parameter for block_id=%lu, buffer=%p, buffer_len=%zu", block_id, buffer, buffer_len);
        return NVM_STATUS_INVALID_PARAM;
    }
    if (buffer_len < g_nvm_block_configs[block_id].size)
    {
        ESP_LOGE(TAG, "NVM_Read: Buffer too small for block %lu (Required: %zu, Provided: %zu)",
                 block_id, g_nvm_block_configs[block_id].size, buffer_len);
        return NVM_STATUS_DATA_TOO_LARGE;
    }

    NVM_Status_t status = NVM_STATUS_ERROR;
    if (xSemaphoreTake(g_nvm_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "NVM_Read: Failed to take mutex.");
        return NVM_STATUS_BUSY;
    }

    // Ensure the block is loaded into RAM (should be done during init, but for robustness)
    if (!g_nvm_runtime_data[block_id].is_loaded)
    {
        status = nvm_load_block_from_nvm(block_id);
        if (status != NVM_STATUS_OK && status != NVM_STATUS_CRC_ERROR)
        {
            ESP_LOGE(TAG, "NVM_Read: Failed to load block %lu before reading. Status: %d", block_id, status);
            xSemaphoreGive(g_nvm_mutex);
            return status;
        }
    }

    memcpy(buffer, g_nvm_runtime_data[block_id].data, g_nvm_block_configs[block_id].size);
    status = NVM_STATUS_OK;

    xSemaphoreGive(g_nvm_mutex);
    return status;
}

NVM_Status_t NVM_Write(uint32_t block_id, const void *data, uint16_t data_len)
{
    if (!g_nvm_initialized)
    {
        return NVM_STATUS_NOT_INITIALIZED;
    }
    if (block_id >= NVM_BLOCK_ID_MAX || data == NULL || data_len == 0)
    {
        ESP_LOGE(TAG, "NVM_Write: Invalid parameter for block_id=%lu, data=%p, data_len=%zu", block_id, data, data_len);
        return NVM_STATUS_INVALID_PARAM;
    }
    if (data_len > g_nvm_block_configs[block_id].size)
    {
        ESP_LOGE(TAG, "NVM_Write: Data too large for block %lu (Max: %zu, Provided: %zu)",
                 block_id, g_nvm_block_configs[block_id].size, data_len);
        return NVM_STATUS_DATA_TOO_LARGE;
    }

    NVM_Status_t status = NVM_STATUS_ERROR;
    if (xSemaphoreTake(g_nvm_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "NVM_Write: Failed to take mutex.");
        return NVM_STATUS_BUSY;
    }

    // Ensure the block is loaded before modifying (to prevent overwriting fresh data)
    if (!g_nvm_runtime_data[block_id].is_loaded)
    {
        status = nvm_load_block_from_nvm(block_id);
        if (status != NVM_STATUS_OK && status != NVM_STATUS_CRC_ERROR)
        {
            ESP_LOGE(TAG, "NVM_Write: Failed to load block %lu before writing. Status: %d", block_id, status);
            xSemaphoreGive(g_nvm_mutex);
            return status;
        }
    }

    // Only mark dirty if data actually changes
    if (memcmp(g_nvm_runtime_data[block_id].data, data, data_len) != 0)
    {
        memcpy(g_nvm_runtime_data[block_id].data, data, data_len);
        g_nvm_runtime_data[block_id].is_dirty = true;
        ESP_LOGD(TAG, "NVM block %lu marked as dirty.", block_id);
    }
    else
    {
        ESP_LOGD(TAG, "NVM_Write: No change detected for block %lu.", block_id);
    }

    status = NVM_STATUS_OK;

    xSemaphoreGive(g_nvm_mutex);
    return status;
}

NVM_Status_t NVM_Commit(void)
{
    if (!g_nvm_initialized)
    {
        return NVM_STATUS_NOT_INITIALIZED;
    }

    NVM_Status_t overall_status = NVM_STATUS_OK;
    bool changes_committed = false;

    if (xSemaphoreTake(g_nvm_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "NVM_Commit: Failed to take mutex.");
        return NVM_STATUS_BUSY;
    }

    for (uint32_t i = 0; i < NVM_BLOCK_ID_MAX; i++)
    {
        if (g_nvm_runtime_data[i].is_dirty)
        {
            ESP_LOGI(TAG, "NVM_Commit: Committing block %lu...", i);
            NVM_Status_t block_status = nvm_save_block_to_nvm(i);
            if (block_status != NVM_STATUS_OK)
            {
                ESP_LOGE(TAG, "NVM_Commit: Failed to commit block %lu. Status: %d", i, block_status);
                overall_status = block_status; // Keep track of the first error
            }
            else
            {
                changes_committed = true;
            }
        }
    }

    xSemaphoreGive(g_nvm_mutex);

    if (!changes_committed && overall_status == NVM_STATUS_OK)
    {
        ESP_LOGI(TAG, "NVM_Commit: No changes to commit.");
        return NVM_STATUS_NO_CHANGES;
    }

    ESP_LOGI(TAG, "NVM_Commit: Operation completed with status %d.", overall_status);
    return overall_status;
}

NVM_Status_t NVM_Format(void)
{
    if (!g_nvm_initialized)
    {
        return NVM_STATUS_NOT_INITIALIZED;
    }

    NVM_Status_t overall_status = NVM_STATUS_OK;

    if (xSemaphoreTake(g_nvm_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "NVM_Format: Failed to take mutex.");
        return NVM_STATUS_BUSY;
    }

    // Erase all relevant sectors (simplified, assuming contiguous NVM area for all blocks)
    // In a real system, you might iterate through sectors covered by NVM_PHYSICAL_BASE_ADDRESS
    // and the total size of all blocks. For simplicity, we just iterate blocks and save defaults.
    // A more robust format would involve erasing the entire NVM partition.

    for (uint32_t i = 0; i < NVM_BLOCK_ID_MAX; i++)
    {
        // Copy default values to RAM buffer
        memcpy(g_nvm_runtime_data[i].data, g_nvm_block_configs[i].default_value_ptr, g_nvm_block_configs[i].size);
        g_nvm_runtime_data[i].is_dirty = true;  // Mark as dirty to ensure it's written
        g_nvm_runtime_data[i].is_loaded = true; // It's now loaded with defaults

        // Save the default block to NVM
        ESP_LOGI(TAG, "NVM_Format: Writing default for block %lu...", i);
        NVM_Status_t block_status = nvm_save_block_to_nvm(i);
        if (block_status != NVM_STATUS_OK)
        {
            ESP_LOGE(TAG, "NVM_Format: Failed to write default for block %lu. Status: %d", i, block_status);
            overall_status = block_status; // Keep track of the first error
        }
    }

    xSemaphoreGive(g_nvm_mutex);

    ESP_LOGI(TAG, "NVM_Format: Operation completed with status %d.", overall_status);
    return overall_status;
}
