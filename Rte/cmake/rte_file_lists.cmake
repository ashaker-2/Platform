# HAL/hal_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from HAL/ to cmake/project_global_settings.cmake
include(../cmake/project_global_settings.cmake)

# --- Component: HAL ---
# Define source files for the HAL component by globbing

# Define configuration source files for the HAL component (if any)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" RTE_SRC_DIR)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" RTE_CFG_DIR)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/inc" RTE_INC_DIR)

# === SOURCES ===
set(RTE_SRC
    "${RTE_SRC_DIR}/Rte.c"
    # "${RTE_CFG_DIR}/Rte_cfg.c"

)

set(RTE_PUBLIC_INC_DIRS
    "${RTE_INC_DIR}"
)

# set(C_INCLUDE_DIRS
#     "${THIRDPARTY_DIR}/newlib/platform_include"
#     "${THIRDPARTY_DIR}/soc/esp32/register"
#     "${THIRDPARTY_DIR}/esp_system/include"
#     "${THIRDPARTY_DIR}/soc/include"
#     "${THIRDPARTY_DIR}/soc/esp32/include"
#     "${THIRDPARTY_DIR}/xtensa/include"
#     "${THIRDPARTY_DIR}/xtensa/esp32/include"
#     "${THIRDPARTY_DIR}/xtensa/deprecated_include"
#     "${THIRDPARTY_DIR}/esp_common/include"
#     "${THIRDPARTY_DIR}/esp_hw_support/include"
# )