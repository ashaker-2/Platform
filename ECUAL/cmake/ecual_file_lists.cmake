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
set(ECUAL_SRCS

    src/*

)
# Define configuration source files for the app_fan component
set(ECUAL_CONFIG_SRCS

    cfg/*
)
# Define public include directories for the app_fan component
set(ECUAL_PUBLIC_INC_DIRS

    inc/*
)
# Define private include directories for the app_fan component (if any)
set(ECUALPRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the app_fan component
set(ECUAL_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the app_fan component (if any)
set(ECUAL_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the app_fan component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(ECUAL_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the app_fan component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(ECUAL_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
