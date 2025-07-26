# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/mcal_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(../cmake/build_options.cmake)

# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
add_project_component_library(MCAL   # Lib_name
    "${MCAL_SRCS}"                # _srcs
    "${MCAL_CONFIG_SRCS}"         # _srcs_cfg
    "${MCAL_PUBLIC_INC_DIRS}"     # _public_inc_dirs (no extra private inc for fan in this example)
    "${MCAL_PUBLIC_REQUIRES }"    # _requires
    "${MCAL_PRIVATE_REQUIRES}"    # _private_requires (no private dependencies for fan in this example)
    "${MCAL_COMPILE_OPTIONS}"     # _compile_options
    "${MCAL_COMPILE_DEFINITIONS}" # _compile_definitions
)
