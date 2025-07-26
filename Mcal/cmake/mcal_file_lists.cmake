# components/component_file_lists.cmake
#
# This file defines the source files, include directories, and configuration
# source files for each component in the project.
# It also includes global project settings.

# Include the global project settings to inherit common options/definitions.
# Path is relative from components/ to cmake/project_global_settings.cmake
include(../../cmake/project_global_settings.cmake)

# --- Component: app_fan ---
# Define source files for the app_fan component
set(MCAL_SRCS

    adc/src/adc.c
    gpio/src/gpio.c
    i2c/src/i2c.c
    pwm/src/pwm.c
    spi/src/spi.c
    timers/src/timers.c
    uart/src/uart.c

)
# Define configuration source files for the app_fan component
set(MCAL_CONFIG_SRCS

    adc/src/adc_cfg.c
    gpio/src/gpio_cfg.c
    i2c/src/i2c_cfg.c
    pwm/src/pwm_cfg.c
    spi/src/spi_cfg.c
    timers/src/timers_cfg.c
    uart/src/uart_cfg.c
)
# Define public include directories for the app_fan component
set(MCAL_PUBLIC_INC_DIRS

    adc/inc/
    gpio/inc/
    i2c/inc/
    pwm/inc/
    spi/inc/
    timers/inc/
    uart/inc/

)
# Define private include directories for the app_fan component (if any)
set(MCAL_PRIVATE_INC_DIRS
    # Example: "private_inc"
)
# Define public dependencies for the app_fan component
set(MCAL_PUBLIC_REQUIRES
    # Example: "some_internal_lib"
)
# Define private dependencies for the app_fan component (if any)
set(MCAL_PRIVATE_REQUIRES
    # Example: "some_internal_lib"
)
# Define compile options for the app_fan component (inherits from GLOBAL_C_COMPILE_OPTIONS)
set(MCAL_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS}
    # -Wno-unused-variable # Example: specific warning disable
)
# Define compile definitions for the app_fan component (inherits from GLOBAL_COMPILE_DEFINITIONS)
set(MCAL_COMPILE_DEFINITIONS
    ${GLOBAL_COMPILE_DEFINITIONS}
)
