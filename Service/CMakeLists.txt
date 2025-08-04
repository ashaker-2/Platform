# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/middle_sw_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(../cmake/build_options.cmake)

# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
add_project_component_library(MiddleSW   # Lib_name
    "${MiddleSW_SRCS}"                # _srcs
    "${MiddleSW_CONFIG_SRCS}"         # _srcs_cfg
    "${MiddleSW_PUBLIC_INC_DIRS}"     # _public_inc_dirs (no extra private inc for fan in this example)
    "${MiddleSW_PUBLIC_REQUIRES }"    # _requires
    "${MiddleSW_PRIVATE_REQUIRES}"    # _private_requires (no private dependencies for fan in this example)
    "${MiddleSW_COMPILE_OPTIONS}"     # _compile_options
    "${MiddleSW_COMPILE_DEFINITIONS}" # _compile_definitions
)
