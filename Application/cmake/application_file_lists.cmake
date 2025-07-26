# components/component_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from components/ to cmake/project_global_settings.cmake
include(../../cmake/project_global_settings.cmake)

# --- Component: app_fan ---
# Define source files for the app_fan component
set(APPLICATION_SRCS

    SystemMonitor/src/SystemMonitor.c
    diagnostic/src/Diag.c
    display/src/display.c
    fan/src/fan.c
    heater/src/heater.c
    lightControl/src/LightControl.c
    lightIndication/src/LightIndication.c
    logger/src/Logger.c
    power/src/Power.c
    pump/src/Pump.c
    systemMgr/src/SystemManager.c
    temperature/src/Temperature.c
    humadity/src/humadity.c
    ventilator/src/ventilator.c

)
# Define configuration source files for the app_fan component
set(APPLICATION_CONFIG_SRCS

    SystemMonitor/src/SystemMonitor_cfg.c
    diagnostic/src/Diag_cfg.c
    display/src/display_cfg.c
    fan/src/fan_cfg.c
    heater/src/heater_cfg.c
    lightControl/src/LightControl_cfg.c
    lightIndication/src/LightIndication_cfg.c
    logger/src/Logger_cfg.c
    power/src/Power_cfg.c
    pump/src/Pump_cfg.c
    systemMgr/src/SystemManager_cfg.c
    temperature/src/Temperature_cfg.c
    humadity/src/humadity_cfg.c
    ventilator/src/ventilator_cfg.c
)
# Define public include directories for the app_fan component
set(APPLICATION_PUBLIC_INC_DIRS
    SystemMonitor/inc/
    cmake/inc/
    common/inc/
    diagnostic/inc/
    display/inc/
    fan/inc/
    heater/inc/
    lightControl/inc/
    lightIndicatioinc/n/
    logger/inc/
    power/inc/
    pump/inc/
    systemMgr/inc/
    temperature/inc/
    ventilator/inc/
)
# Define private include directories for the app_fan component (if any)
set(APPLICATIONPRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the app_fan component
set(APPLICATION_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the app_fan component (if any)
set(APPLICATION_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the app_fan component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(APPLICATION_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the app_fan component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(APPLICATION_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
    -DFAN_DEBUG_MODE
)
