/**
 * @file nvm_cfg.c
 * @brief Non-Volatile Memory (NVM) Service - Configuration Implementation.
 *
 * This file implements the static configuration parameters for the NVM service.
 * It defines the properties of each NVM data block, including its size
 * and default values. These configurations are used by the `nvm.c`
 * implementation to manage persistent storage.
 *
 * IMPORTANT:
 * 1. Ensure the `size` of each block matches the actual size of its
 * corresponding default value structure/array.
 * 2. The sum of all block sizes (plus any overhead for metadata/CRC)
 * must fit within the allocated physical NVM area.
 * 3. The `NVM_MAX_BLOCK_SIZE_BYTES` in `nvm_cfg.h` must be large enough
 * to accommodate the largest block defined here.
 */

#include "nvm_cfg.h"
#include "nvm.h" // For NVM_BlockId_t

// --- Define Default Values for each NVM Block ---

/**
 * @brief Default values for NVM_BLOCK_ID_SYSTEM_SETTINGS.
 */
typedef struct
{
    uint32_t magic_word;
    uint16_t device_id;
    uint8_t  operation_mode;
    bool     enable_feature_x;
    uint8_t  reserved[10]; // Padding to ensure fixed size
} SystemSettings_t;

static const SystemSettings_t s_default_system_settings = {
    .magic_word = 0xDEADBEEF,
    .device_id = 0x1234,
    .operation_mode = 1,
    .enable_feature_x = true,
    .reserved = {0}
};

/**
 * @brief Default values for NVM_BLOCK_ID_CALIBRATION_DATA.
 */
typedef struct
{
    float temp_offset;
    float hum_gain;
    int16_t pressure_bias;
    uint8_t reserved[8];
} CalibrationData_t;

static const CalibrationData_t s_default_calibration_data = {
    .temp_offset = 0.5f,
    .hum_gain = 1.02f,
    .pressure_bias = 10,
    .reserved = {0}
};

/**
 * @brief Default values for NVM_BLOCK_ID_DEVICE_INFO.
 */
typedef struct
{
    char serial_number[16];
    uint8_t hw_version_major;
    uint8_t hw_version_minor;
    uint8_t fw_version_major;
    uint8_t fw_version_minor;
} DeviceInfo_t;

static const DeviceInfo_t s_default_device_info = {
    .serial_number = "EMCS-0001",
    .hw_version_major = 1,
    .hw_version_minor = 0,
    .fw_version_major = 1,
    .fw_version_minor = 0
};

/**
 * @brief Default values for NVM_BLOCK_ID_LOG_POINTER.
 */
static const uint32_t s_default_log_pointer = 0; // Start logging from address 0

// --- Global Array of NVM Block Configurations ---

/**
 * @brief The global array defining all NVM blocks and their properties.
 *
 * The order of blocks in this array should match the order of `NVM_BlockId_t` enum.
 */
const NVM_BlockConfig_t g_nvm_block_configs[NVM_BLOCK_ID_MAX] = {
    [NVM_BLOCK_ID_SYSTEM_SETTINGS] = {
        .id = NVM_BLOCK_ID_SYSTEM_SETTINGS,
        .size = sizeof(SystemSettings_t),
        .default_value_ptr = &s_default_system_settings
    },
    [NVM_BLOCK_ID_CALIBRATION_DATA] = {
        .id = NVM_BLOCK_ID_CALIBRATION_DATA,
        .size = sizeof(CalibrationData_t),
        .default_value_ptr = &s_default_calibration_data
    },
    [NVM_BLOCK_ID_DEVICE_INFO] = {
        .id = NVM_BLOCK_ID_DEVICE_INFO,
        .size = sizeof(DeviceInfo_t),
        .default_value_ptr = &s_default_device_info
    },
    [NVM_BLOCK_ID_LOG_POINTER] = {
        .id = NVM_BLOCK_ID_LOG_POINTER,
        .size = sizeof(uint32_t),
        .default_value_ptr = &s_default_log_pointer
    }
};

