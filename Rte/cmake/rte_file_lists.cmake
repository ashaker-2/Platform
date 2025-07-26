# components/component_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from components/ to cmake/project_global_settings.cmake
include(../../cmake/project_global_settings.cmake)

# --- Component: RTE ---
# Define source files for the RTE component
set(RTE_SRCS

    src/*

)
# Define configuration source files for the RTE component
set(RTE_CONFIG_SRCS

    cfg/*
)
# Define public include directories for the RTE component
set(RTE_PUBLIC_INC_DIRS

    inc/*
)
# Define private include directories for the RTE component (if any)
set(RTEPRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the RTE component
set(RTE_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the RTE component (if any)
set(RTE_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the RTE component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(RTE_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the RTE component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(RTE_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
