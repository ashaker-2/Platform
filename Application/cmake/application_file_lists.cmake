# components/component_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from components/ to cmake/project_global_settings.cmake
# include(../cmake/project_global_settings.cmake)

# --- Application Component  ---
# Define source files for the Application component by globbing

# === CONFIGURE PATH ===

file(TO_CMAKE_PATH "${APP_DIR}/temphum"      TEMPHUM_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/fan"              FAN_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/heater"           HEAT_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/humadity"         HUM_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/display"          DISP_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/diagnostic"       DIAG_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/lightControl"     LigCtrl_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/lightIndication"  LigIdn_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/logger"           Log_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/OTA"              OTA_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/power"            PWR_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/pump"             PUMP_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/systemMgr"        SysMgr_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/SystemMonitor"    SysMon_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/SystemStartup"    SysStart_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/ventilator"       VEN_DIR)
file(TO_CMAKE_PATH "${APP_DIR}/common"           COMMON_DIR)



# --- Application SWC ---
# Define source files for the  component
set(APPLICATION_SRCS

    "${SysStart_DIR}/src/startup.c"
    "${TEMPHUM_DIR}/src/temphumctrl.c"
    "${TEMPHUM_DIR}/src/temphumctrl_cfg.c"
    "${FAN_DIR}/src/fanctrl.c"
    "${FAN_DIR}/src/fanctrl_cfg.c"
    "${HEAT_DIR}/src/Heaterctrl.c"
    "${HEAT_DIR}/src/Heaterctrl_cfg.c"
    "${DISP_DIR}/src/char_display.c"
    "${DISP_DIR}/src/char_display_cfg.c"
    # "${DIAG_DIR}/src/Diag_cfg.c"
    # "${DIAG_DIR}/src/Diag.c"
    "${LigCtrl_DIR}/src/lightctrl.c"
    "${LigCtrl_DIR}/src/lightctrl_cfg.c"
    "${LigIdn_DIR}/src/lightind.c"
    "${LigIdn_DIR}/src/lightind_cfg.c"
    "${PWR_DIR}/src/Power.c"
    "${PWR_DIR}/src/Power_cfg.c"
    "${PUMP_DIR}/src/pumpctrl.c"
    "${PUMP_DIR}/src/pumpctrl_cfg.c"
    "${SysMgr_DIR}/src/sys_mgr.c"
    "${SysMgr_DIR}/src/sys_mgr_cfg.c"
    "${SysMon_DIR}/src/system_monitor.c"
    "${SysMon_DIR}/src/system_monitor_cfg.c"
    "${VEN_DIR}/src/ventctrl.c"
    "${VEN_DIR}/src/ventctrl_cfg.c"
    # "${Log_DIR}/src/logger.c"
    # "${Log_DIR}/src/logger_cfg.c"
    # "${OTA_DIR}/src/ota.c"    
)

# Define public include directories for the component
set(APPLICATION_PUBLIC_INC_DIRS
    
    "${TEMPHUM_DIR}/inc"
    "${FAN_DIR}/inc"
    "${HEAT_DIR}/inc"
    "${DISP_DIR}/inc"
    "${DIAG_DIR}/inc"
    "${LigCtrl_DIR}/inc"
    "${LigIdn_DIR}/inc"
    "${Log_DIR}/inc"
    "${OTA_DIR}/inc"
    "${PWR_DIR}/inc"
    "${PUMP_DIR}/inc"
    "${SysMgr_DIR}/inc"
    "${SysMon_DIR}/inc"
    "${SysStart_DIR}/inc"
    "${VEN_DIR}/inc"
)

# Define public dependencies for the  component
set(APPLICATION_PUBLIC_REQUIRES
    
)
# Define private dependencies for the  component (if any)
set(APPLICATION_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the  component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(APPLICATION_COMPILE_OPTIONS
    
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the  component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(APPLICATION_COMPILE_DEFINITIONS
    # ${GLOBAL_COMPILE_DEFINITIONS}
)


