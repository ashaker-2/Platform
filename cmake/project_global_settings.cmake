# cmake/global_settings.cmake
#
# This file defines global build settings for the entire project.
# It is included by the top-level CMakeLists.txt and individual component
# option files to provide a consistent baseline.

# Define global C compiler options
set(GLOBAL_C_COMPILE_OPTIONS
    -Os                   # Optimize for size
    -Wall                 # Enable all warnings
    -Wextra               # Enable extra warnings
    -fno-builtin          # Avoid built-in functions
    -ffunction-sections   # Place each function in its own section
    -fdata-sections       # Place each data item in its own section
    # Add any other universally applied C flags here
)

# Define global C++ compiler options (if you have C++ in your project)
set(GLOBAL_CXX_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS} # Inherit C flags
    -fno-exceptions       # Disable exceptions for embedded
    -fno-rtti             # Disable RTTI for embedded
    # Add any other universally applied C++ flags here
)

# Define global compile definitions (macros applied to all source files)
set(GLOBAL_COMPILE_DEFINITIONS
    -DPROJECT_VERSION="1.0.0"   # Example: Global project version
    -DLOG_LEVEL_INFO            # Example: Default logging level
    -DCONFIG_SOME_FEATURE_ENABLED # Example: A globally enabled feature
    # Add any other universally applied definitions here
)

# Define global include directories that all components might need
# This might include paths to common utility headers or SDK headers
# (Note: For system-level headers, the toolchain/SDK setup often handles this.)
set(GLOBAL_INCLUDE_DIRS
    # "${CMAKE_CURRENT_SOURCE_DIR}/components/common/inc" # Example: if you have a common app/bsw header folder
    # "${CMAKE_CURRENT_SOURCE_DIR}/external_libs/freertos/inc" # Example: FreeRTOS global headers
)

# You can also set global linker options here if they apply to the entire final executable.
# set(GLOBAL_LINK_OPTIONS "-Wl,--gc-sections")

# These variables will be available to any CMakeLists.txt or .cmake file
# that includes this file.