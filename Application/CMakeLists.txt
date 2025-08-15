# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/application_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(../cmake/build_options.cmake)

# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
add_library(Application   # Lib_name
    ${APPLICATION_SRCS}                 # _srcs
)  


target_include_directories(Application PRIVATE
    
    ${APPLICATION_PUBLIC_INC_DIRS}   # _public_inc_dirs (no extra private inc for fan in this example)
    ${GLOBAL_COMMON_INCLUDE_DIRS}          # _requires
    ${GLOBAL_HAL_INCLUDE_DIRS}    # _private_requires (no private dependencies for fan in this example)
    ${GLOBAL_RTE_INCLUDE_DIRS}     # _compile_options
    ${GLOBAL_SERVICE_INCLUDE_DIRS} # _compile_definitions
)
