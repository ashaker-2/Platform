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
set(MiddleSW_SRCS

    os/src/*
    modbus/src/*

)
# Define configuration source files for the app_fan component
set(MiddleSW_CONFIG_SRCS

    os/cfg/*
    modbus/cfg/*
)
# Define public include directories for the app_fan component
set(MiddleSW_PUBLIC_INC_DIRS

    os/inc/*
    modbus/inc/*
)
# Define private include directories for the app_fan component (if any)
set(MiddleSWPRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the app_fan component
set(MiddleSW_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the app_fan component (if any)
set(MiddleSW_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the app_fan component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(MiddleSW_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the app_fan component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(MiddleSW_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
