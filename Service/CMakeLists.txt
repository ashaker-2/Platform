# components/CMakeLists.txt
#
# This CMakeLists.txt file is responsible for adding all custom software components
# within the 'components' directory as subdirectories to the build system.

# Include the common component macro definitions.
# Path is relative from components/ to components/component_macros.cmake
include(cmake/Service_file_lists.cmake)

# Include component-specific build options.
# Path is relative to this CMakeLists.txt file.
include(./../cmake/build_options.cmake)


# Call the macro to define the 'app_fan' library using the settings from fan_build_options.cmake
# add_library(SERVICE   # Lib_name
#     "${SERVICE_SRCS}"                # _srcs
#     "${SERVICE_CONFIG_SRCS}"         # _srcs_cfg
#     "${SERVICE_PUBLIC_INC_DIRS}"     # _public_inc_dirs (no extra private inc for fan in this example)
#     "${SERVICE_PUBLIC_REQUIRES }"    # _requires
#     "${SERVICE_PRIVATE_REQUIRES}"    # _private_requires (no private dependencies for fan in this example)
#     "${SERVICE_COMPILE_OPTIONS}"     # _compile_options
#     "${SERVICE_COMPILE_DEFINITIONS}" # _compile_definitions
#     "${FREERTOS_SRC}"
# )

# message(STATUS "MY_VAR = ${SERVICE_PUBLIC_INC_DIRS}")

add_library(SERVICE
    ${SERVICE_SRCS}
    # ${OS_ASM}   
    
)


target_include_directories(SERVICE PRIVATE
    ${OS_INCLUDE_DIRS_GENERIC}   
)

target_include_directories(SERVICE PRIVATE
    ${C_INCLUDE_DIRS}   
)




