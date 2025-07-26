# /your_project_name/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Define your project name
project(DemoFS C)

# Include global project settings.
# This makes variables like GLOBAL_C_COMPILE_OPTIONS available throughout the project.
include(cmake/project_global_settings.cmake)

# --- Toolchain setup (highly simplified, real one is complex) ---
# This part would tell CMake to use your cross-compiler for the ESP32.
# This is highly dependent on your actual toolchain and build environment.
# Example:
# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR xtensa-esp32)
# set(CMAKE_C_COMPILER /path/to/xtensa-esp32-elf/bin/xtensa-esp32-elf-gcc)
# # ... (rest of toolchain setup) ...

# Apply global compile options and definitions to the entire project (e.g., to the executable itself)
# This ensures a baseline, which can then be overridden/extended by component-specific options.
add_compile_options(${GLOBAL_C_COMPILE_OPTIONS})
add_definitions(${GLOBAL_COMPILE_DEFINITIONS})
# Add global include directories
# include_directories(${GLOBAL_INCLUDE_DIRS})


# Add the 'components' directory as a subdirectory.
add_subdirectory(Mcal)
add_subdirectory(ECUAL)
add_subdirectory(MiddleSW)
add_subdirectory(Rte)
add_subdirectory(Application)


# Define the main executable
# The 'main' component should define its own sources and potentially link to other libraries.
# Here, we just link the 'main' component library and other necessary system libraries.
add_executable(${PROJECT_NAME}.elf
    # The 'main' component's main.c is typically here
    Application/SystemStartup/src/main.c
)

# Link all the component libraries and any necessary external libraries to the main executable.
# The libraries defined by your components (e.g., app_fan, mcal_gpio) will be available here.
target_link_libraries(${PROJECT_NAME}.elf
    PUBLIC
        MCAL           # Link to the application library
        ECUAL          # Link to the ECUAL library
        MiddleSW       # Link to the MiddleSW library
        RTE            # Link to the RTE library
        Application    # Link to the Application library
)

# ... (remaining parts of top-level CMakeLists.txt like flashing commands)