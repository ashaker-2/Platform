# # Service/Service_file_lists.cmake
# #
# # This file defines the source files, include directories, and configuration
# # source files for each component in the project.
# # It also includes global project settings.

# # Include the global project settings to inherit common options/definitions.
# # Path is relative from Service/ to cmake/project_global_settings.cmake
# include(../cmake/project_global_settings.cmake)

# --- Service Component  ---
# Define source files for the Service component by globbing

# === CONFIGURE PATH ===

# file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/OS" OS_DIR)
# file(TO_CMAKE_PATH "${OS_DIR}/FreeRTOS-Kernel-SMP" OS_KERNEL_DIR)
# file(TO_CMAKE_PATH "${OS_KERNEL_DIR}/portable/xtensa" OS_PORT_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/../Thirdparty" THIRDPARTY_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/ComM" COMM_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/Modbus" MODBUS_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/NvM" NVM_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/OTA" OTA_DIR)
file(TO_CMAKE_PATH "${Service_DIR}/Security" SECURITY_DIR)




# === SOURCES ===
# set(OS_SRC
#     "${OS_KERNEL_DIR}/list.c"
#     "${OS_KERNEL_DIR}/queue.c"
#     "${OS_KERNEL_DIR}/tasks.c"
#     "${OS_KERNEL_DIR}/timers.c"
#     "${OS_KERNEL_DIR}/event_groups.c"
#     "${OS_KERNEL_DIR}/stream_buffer.c"
#     "${OS_PORT_DIR}/port.c"

#     "${OS_PORT_DIR}/xtensa_init.c"
#     "${OS_PORT_DIR}/xtensa_overlay_os_hook.c"
#     "${OS_DIR}/heap_idf.c"
#     "${OS_DIR}/app_startup.c"
#     "${OS_DIR}/port_common.c"
#     "${OS_DIR}/port_systick.c"
#     "${OS_DIR}/esp_additions/freertos_compatibility.c"
#     "${OS_DIR}/esp_additions/idf_additions_event_groups.c"
#     "${OS_DIR}/esp_additions/idf_additions.c"
# )

# set(OS_ASM
#     "${OS_PORT_DIR}/portasm.S"
# )

# # === INCLUDES ===
# # Define C-specific include directories (these contain C headers the assembler can't parse)
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

# # Define generic/assembly-safe include directories
# set(OS_INCLUDE_DIRS_GENERIC
#     "${OS_DIR}"
#     "${OS_DIR}/config/include"
#     "${OS_DIR}/config/include/freertos"
#     "${OS_DIR}/config/xtensa/include"
#     "${OS_KERNEL_DIR}/include/freertos"
#     "${OS_KERNEL_DIR}/include"
#     "${OS_PORT_DIR}/include"
#     "${OS_PORT_DIR}/include/freertos"
#     "${OS_DIR}/esp_additions/include"
#     "${OS_KERNEL_DIR}/portable/linux/include"
#     "${THIRDPARTY_DIR}/esp_rom/include"
#     "${THIRDPARTY_DIR}/log/include"
#     "${THIRDPARTY_DIR}/heap/include"
# )

# set(SERVICE_SRCS
    
#     ${OS_SRC}

# )


# Define public include directories for the  component
set(SERVICE_PUBLIC_INC_DIRS
    
    

)
# Define private include directories for the component (if any)
set(SERVICE_PRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the component
set(SERVICE_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the  component (if any)
set(SERVICE_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the  component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(SERVICE_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the  component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(SERVICE_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)


# # Define configuration source files for the Service component (if any)
# file(GLOB SERVICE_SRCS_DIRS "${CMAKE_CURRENT_LIST_DIR}/Service/src/*.c") # Assuming src/ for sources


# # Define configuration source files for the Service component (if any)
# file(GLOB SERVICE_CONFIG_DIRS "${CMAKE_CURRENT_LIST_DIR}/Service/cfg/*.c") # Assuming cfg/ for config

# # Define public include directories for the Service component
# # This sets the actual path to the 'inc' directory.
# set(SERVICE_PUBLIC_INC_DIRS
#     "${CMAKE_CURRENT_LIST_DIR}/Service/inc"
# )
# # Example if Service/inc/ is relative to THIS file's directory (components/):
# # set(SERVICE_PUBLIC_INC_DIRS "Service/inc")
# # Define private include directories for the Service component (if any)
# set(SERVICE_PRIVATE_INC_DIRS
#     "${CMAKE_CURRENT_LIST_DIR}/Service/inc"
# )
# # Define public dependencies for the Service component
# set(SERVICE_PUBLIC_REQUIRES
#     # Example: "some_internal_lib"
# )
# # Define private dependencies for the Service component (if any)
# set(SERVICE_PRIVATE_REQUIRES
#     # Example: "some_internal_lib"
# )
# # Define compile options for the Service component (inherits from GLOBAL_C_COMPILE_OPTIONS)
# set(SERVICE_COMPILE_OPTIONS
#     ${GLOBAL_C_COMPILE_OPTIONS}
#     # -Wno-unused-variable # Example: specific warning disable
# )
# # Define compile definitions for the Service component (inherits from GLOBAL_COMPILE_DEFINITIONS)
# set(SERVICE_COMPILE_DEFINITIONS
#     ${GLOBAL_COMPILE_DEFINITIONS}
# )
