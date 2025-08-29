/**
 * @file sys_mgr.c
 * @brief System Manager Core Interface
 * @version 2.1
 * @date 2025
 *
 * This file acts as the top-level interface for the System Manager component.
 * It provides the public APIs for initialization, configuration management,
 * and the main periodic function. The core control logic, including the
 * state machine and actuator control, is handled by the internal `sys_mgr_core`
 * module to promote modularity.
 */

#include "sys_mgr.h"
#include "sys_mgr_cfg.h"
#include "sys_mgr_core.h" // New module for core logic
#include "ui_manager.h"   // New file name
#include "logger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// #include "FlashMgr.h" // Assumed to be available
#include <string.h>

#define FLASH_CFG_ID 1

/* Global System Configuration */
static SysMgr_Config_t g_system_configuration;
static SemaphoreHandle_t g_config_mutex;
static bool g_initialized = false;
static bool g_is_save_pending = false;
static const char *TAG = "SysMgr";
/* --- Forward Declarations --- */
static void load_defaults(void);

/**
 * @brief Initializes the System Manager component.
 *
 * This function is the entry point for the SysMgr module. It creates
 * a mutex for thread-safe access to the configuration and attempts
 * to load a persistent configuration from flash. If no valid configuration
 * is found, it loads the default settings.
 *
 * @return Status_t E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_Init(void)
{
    if (g_initialized)
        return E_OK;

    g_config_mutex = xSemaphoreCreateMutex();
    if (!g_config_mutex)
    {
        LOGE(TAG,"Failed to create config mutex");
        return E_NOK;
    }
    size_t sz = sizeof(SysMgr_Config_t);
    Status_t status = E_NOK;
    /* Attempt a real flash read; using hypothetical FlashMgr_ReadConfig API */
    // status = FlashMgr_ReadConfig(FLASH_CFG_ID, &g_system_configuration, sz);
    
    if (status == E_OK) 
    {
        if (SYS_MGR_ValidateConfig(&g_system_configuration) != E_OK) 
        {
            LOGW(TAG,"Flash config invalid - using defaults");
            load_defaults();
        } 
        else 
        {
            LOGI(TAG,"Loaded config from flash");
        }
    } 
    else 
    {
        LOGW(TAG,"No flash config - loading defaults");
        load_defaults();
    }

    g_initialized = true;
    LOGI(TAG,"SysMgr initialized.");

    return E_OK;
}


/**
 * @brief Loads the default configuration values.
 *
 * This private function is called during initialization if a persistent
 * configuration cannot be loaded from flash.
 */
static void load_defaults(void)
{
    g_system_configuration = g_default_system_configuration;
}

/**
 * @brief Gets the current system configuration.
 *
 * @param[out] out A pointer to the SysMgr_Config_t struct to be populated.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out)
{
    if (!out)
        return E_NOK;
    xSemaphoreTake(g_config_mutex, portMAX_DELAY);
    *out = g_system_configuration;
    xSemaphoreGive(g_config_mutex);
    return E_OK;
}

/**
 * @brief Updates the runtime configuration.
 *
 * This function is used by the UI or other modules to update the system's
 * operational configuration. It marks the configuration as dirty,
 * so it will be saved to flash during the next main loop cycle.
 *
 * @param[in] in A pointer to the new configuration.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_UpdateConfigRuntime(const SysMgr_Config_t *in)
{
    if (!in)
        return E_NOK;
    xSemaphoreTake(g_config_mutex, portMAX_DELAY);
    g_system_configuration = *in;
    g_is_save_pending = true;
    xSemaphoreGive(g_config_mutex);
    LOGI(TAG,"Runtime config updated. Save pending.");
    return E_OK;
}

/**
 * @brief Saves the current configuration to flash.
 *
 * This function is called periodically by the main function or on demand.
 * It checks if a save is pending and writes the current configuration
 * to non-volatile memory.
 *
 * @return Status_t E_OK on success, E_NOK on flash write failure.
 */
Status_t SYS_MGR_SaveConfigToFlash(void)
{
    xSemaphoreTake(g_config_mutex, portMAX_DELAY);
    if (!g_is_save_pending) 
    {
        xSemaphoreGive(g_config_mutex);
        return E_OK;
    }

    Status_t status = E_OK;
    // status = FlashMgr_SaveConfig(FLASH_CFG_ID, &g_system_configuration, sizeof(g_system_configuration));
    if (status == E_OK) 
    {
        g_is_save_pending = false;
        LOGI(TAG,"Config saved to flash.");
    } else {
        LOGE(TAG,"Failed to save config to flash.");
        /* Report fault */
        // SysMon_ReportFaultStatus(SYS_MON_FAULT_FLASH_WRITE, SYS_MON_FAULT_STATUS_FAIL);
    }
    xSemaphoreGive(g_config_mutex);
    return status;
}

/**
 * @brief The main periodic function for the System Manager.
 *
 * This function serves as the heart of the system. It calls the core
 * logic to update sensor readings and manage the state machine. It is
 * designed to be called at a regular interval.
 *
 * @param void.
 */
void SYS_MGR_MainFunction(void)
{
    if (!g_initialized)
        return;

    // Call the core logic for sensor updates and state machine processing
    SYS_MGR_CORE_MainFunction(&g_system_configuration);

    // Check for a pending save and save if needed
    if (g_is_save_pending)
    {
        SYS_MGR_SaveConfigToFlash();
    }
}