# components/app/fan/CMakeLists.txt
#
# This CMakeLists.txt defines the 'app_fan' component library,
# utilizing the common macros and component-specific build options.

# Include the component macro definitions.
# Path is relative from components/app/fan to components/component_macros.cmake
include(../../component_macros.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(fan_build_options.cmake)

# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
add_project_component_library(
    app_fan                                   # _name
    "src/app_fan.c src/fan_config.c"          # _srcs
    "inc"                                     # _public_inc_dirs
    ""                                        # _private_inc_dirs (no extra private inc for fan in this example)
    "ecual;app_diagnostics;app_logger;rte"    # _requires
    ""                                        # _private_requires (no private dependencies for fan in this example)
    "${FAN_COMPILE_OPTIONS}"                  # _compile_options
    "${FAN_COMPILE_DEFINITIONS}"              # _compile_definitions
)