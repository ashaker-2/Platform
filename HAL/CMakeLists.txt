# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/hal_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(../cmake/build_options.cmake)

# Call the macro to define the 'HAL' library using the settings from fan_build_options.cmake
add_library(HAL   # Lib_name
    ${HAL_SRCS}                # _srcs
)

target_include_directories(HAL PRIVATE
    ${HAL_PUBLIC_INC_DIRS}         # _srcs_cfg
    ${GLOBAL_APP_INCLUDE_DIRS}     # _public_inc_dirs (no extra private inc for fan in this example) 
    ${GLOBAL_COMMON_INCLUDE_DIRS}     # _public_inc_dirs (no extra private inc for fan in this example) 
    ${GLOBAL_SERVICE_INCLUDE_DIRS} 
)
