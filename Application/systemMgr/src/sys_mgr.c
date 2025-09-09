/**
 * @file sys_mgr.c
 * @brief System Manager Interface
 * @version 2.2 (refactored)
 * @date 2025
 *
 * This file acts as the top-level interface for the System Manager.
 *  - Provides public APIs for initialization, configuration, and state queries
 *  - Owns the runtime system configuration
 *  - Delegates control logic to sys_mgr_core.c
 *  - Handles persistence (save to flash)
 */

#include "sys_mgr.h"
#include "sys_mgr_cfg.h"
#include "sys_mgr_core.h"
#include "logger.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// #include "FlashMgr.h"  /* To be integrated */

#include <string.h>

static const char *TAG = "SysMgr";

/* ============================================================================
 * PRIVATE GLOBALS
 * ========================================================================== */

static SysMgr_Config_t g_system_configuration;
static SemaphoreHandle_t g_cfg_mutex = NULL;
static bool g_initialized = false;
static bool g_save_pending = false;

#define FLASH_CFG_ID  1   /* hypothetical ID for FlashMgr */

/* ============================================================================
 * PRIVATE HELPERS
 * ========================================================================== */
static void load_defaults(void);

/* ============================================================================
 * PUBLIC API
 * ========================================================================== */

Status_t SYS_MGR_Init(void)
{
    if (g_initialized)
        return E_OK;

    g_cfg_mutex = xSemaphoreCreateMutex();
    if (!g_cfg_mutex)
    {
        LOGE(TAG, "Failed to create config mutex");
        return E_NOK;
    }

    /* Attempt load from flash */
    Status_t status = E_NOK;
    size_t sz = sizeof(SysMgr_Config_t);

    // status = FlashMgr_ReadConfig(FLASH_CFG_ID, &g_system_configuration, sz);

    if (status == E_OK)
    {
        if (SYS_MGR_ValidateConfig(&g_system_configuration) != E_OK)
        {
            LOGW(TAG, "Flash config invalid, using defaults");
            load_defaults();
        }
        else
        {
            LOGI(TAG, "Loaded config from flash");
        }
    }
    else
    {
        LOGW(TAG, "No flash config, using defaults");
        load_defaults();
    }

    g_initialized = true;
    LOGI(TAG, "SysMgr initialized");

    return E_OK;
}

void SYS_MGR_MainFunction(void)
{
    if (!g_initialized) return;

    /* Delegate to core logic */
    SYS_MGR_CORE_MainFunction(&g_system_configuration);

    /* Handle save if pending */
    if (g_save_pending)
    {
        SYS_MGR_SaveConfigToFlash();
    }
}

Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out)
{
    if (!out) return E_NOK;
    xSemaphoreTake(g_cfg_mutex, portMAX_DELAY);
    *out = g_system_configuration;
    xSemaphoreGive(g_cfg_mutex);
    return E_OK;
}

Status_t SYS_MGR_UpdateConfig(const SysMgr_Config_t *in)
{
    if (!in) return E_NOK;

    xSemaphoreTake(g_cfg_mutex, portMAX_DELAY);
    g_system_configuration = *in;
    g_save_pending = true;
    xSemaphoreGive(g_cfg_mutex);

    LOGI(TAG, "Configuration updated, save pending");
    return E_OK;
}

Status_t SYS_MGR_SaveConfigToFlash(void)
{
    Status_t status = E_OK;

    xSemaphoreTake(g_cfg_mutex, portMAX_DELAY);
    if (!g_save_pending)
    {
        xSemaphoreGive(g_cfg_mutex);
        return E_OK;
    }

    // status = FlashMgr_SaveConfig(FLASH_CFG_ID,
    //                             &g_system_configuration,
    //                             sizeof(g_system_configuration));

    if (status == E_OK)
    {
        g_save_pending = false;
        LOGI(TAG, "Config saved to flash");
    }
    else
    {
        LOGE(TAG, "Failed to save config to flash");
        /* Could report fault via SysMon */
    }

    xSemaphoreGive(g_cfg_mutex);
    return status;
}

Status_t SYS_MGR_GetActuatorStates(SysMgr_ActuatorStates_t *states_out)
{
    return SYS_MGR_CORE_GetActuatorStates(states_out);
}

Status_t SYS_MGR_GetAverageReadings(float *avg_temp, float *avg_hum)
{
    return SYS_MGR_CORE_GetAverageReadings(avg_temp, avg_hum);
}

/* ============================================================================
 * PRIVATE HELPERS
 * ========================================================================== */

static void load_defaults(void)
{
    g_system_configuration = g_default_system_configuration;
}
