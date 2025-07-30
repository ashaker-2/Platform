# components/component_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from components/ to cmake/project_global_settings.cmake
include(../../cmake/project_global_settings.cmake)

# --- Component: ECUAL ---
# Define source files for the ECUAL component by globbing

# Define configuration source files for the ECUAL component (if any)
file(GLOB ECUAL_SRCS_DIRS "${CMAKE_CURRENT_LIST_DIR}/ecual/src/*.c") # Assuming src/ for sources


# Define configuration source files for the ECUAL component (if any)
file(GLOB ECUAL_CONFIG_DIRS "${CMAKE_CURRENT_LIST_DIR}/ecual/cfg/*.c") # Assuming cfg/ for config

# Define public include directories for the ECUAL component
# This sets the actual path to the 'inc' directory.
set(ECUAL_PUBLIC_INC_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/ecual/inc"
)
# Example if ecual/inc/ is relative to THIS file's directory (components/):
# set(ECUAL_PUBLIC_INC_DIRS "ecual/inc")
# Define private include directories for the ECUAL component (if any)
set(ECUAL_PRIVATE_INC_DIRS
    # Example: "${CMAKE_CURRENT_LIST_DIR}/ecual/private_inc"
)
# Define public dependencies for the ECUAL component
set(ECUAL_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the ECUAL component (if any)
set(ECUAL_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the ECUAL component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(ECUAL_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the ECUAL component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(ECUAL_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
