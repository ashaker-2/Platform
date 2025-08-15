# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/rte_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(../cmake/build_options.cmake)

# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
add_library(RTE   # Lib_name
    ${RTE_SRC}                # _srcs
    # "${RTE_CONFIG_SRCS}"         # _srcs_cfg

)


target_include_directories(RTE PRIVATE
    ${RTE_PUBLIC_INC_DIRS} 
    ${GLOBAL_SERVICE_INCLUDE_DIRS}
    ${GLOBAL_APP_INCLUDE_DIRS} 
    ${GLOBAL_HAL_INCLUDE_DIRS} 
    ${GLOBAL_COMMON_INCLUDE_DIRS}
    
)