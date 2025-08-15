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
file(GLOB HAL_SRCS "${HAL_DIR}/src/*.c") # Assuming src/ for sources

# Define public include directories for the HAL component
# This sets the actual path to the 'inc' directory.
set(HAL_PUBLIC_INC_DIRS
    "${HAL_DIR}/inc"
)
# Example if HAL/inc/ is relative to THIS file's directory (components/):
# set(HAL_PUBLIC_INC_DIRS "HAL/inc")
# Define private include directories for the HAL component (if any)
set(HAL_PRIVATE_INC_DIRS
    
)
# Define public dependencies for the HAL component
set(HAL_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the HAL component (if any)
set(HAL_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the HAL component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(HAL_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the HAL component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(HAL_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
