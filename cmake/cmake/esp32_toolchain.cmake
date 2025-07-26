# cmake/esp32_toolchain.cmake
#
# CMake toolchain file for ESP32 (xtensa-esp32-elf-gcc)

# --- Define Target System ---
set(CMAKE_SYSTEM_NAME Generic) # Target OS is not Linux/Windows/etc., but an embedded system
set(CMAKE_SYSTEM_PROCESSOR xtensa) # The core architecture is Xtensa (specifically Xtensa LX6/LX7 for ESP32)
set(CMAKE_SYSTEM_VERSION 1) # Arbitrary version for a generic system

# --- Toolchain Paths ---
# IMPORTANT: YOU MUST ADJUST THIS PATH TO YOUR ACTUAL TOOLCHAIN LOCATION!
# This path is usually inside your ESP-IDF installation, e.g.,
# C:/Users/YourUser/esp/tools/xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf
# /opt/esp/tools/xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf
set(TOOLCHAIN_PATH "/path/to/your/esp/tools/xtensa-esp32-elf/esp-xxxx/xtensa-esp32-elf")

# --- Define Compilers and Utilities ---
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-gcc") # GCC handles assembly too
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-objcopy")
set(CMAKE_SIZE "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-size")
set(CMAKE_AR "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_PATH}/bin/xtensa-esp32-elf-ranlib")

# --- Default Compiler Flags (often defined in your project_global_settings.cmake) ---
# These are basic flags. More specific flags (like CPU-specific, optimization)
# might be in your project_global_settings.cmake or passed per-target.
# The ESP-IDF build system adds many more flags automatically.
set(CMAKE_C_FLAGS_INIT "-mlongcalls -mtext-section-literals -falign-functions=16 -std=gnu11")
set(CMAKE_CXX_FLAGS_INIT "-mlongcalls -mtext-section-literals -falign-functions=16 -fno-rtti -fno-exceptions -std=gnu++11")
set(CMAKE_ASM_FLAGS_INIT "-mlongcalls -mtext-section-literals")

# --- Linker Configuration ---
# This is crucial for embedded systems. You need a linker script (.ld file).
# This example points to a conceptual linker script location.
# You would get this from ESP-IDF (e.g., esp32_out.ld from build output)
# or create your own if building bare-metal.
set(ESP32_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker_scripts/esp32_flash.ld")

set(CMAKE_EXE_LINKER_FLAGS_INIT "
    -nostdlib
    -Wl,--gc-sections
    -Wl,--cref
    -Wl,--defsym=__ESP32_FLASH_SIZE=4M # Example: Define flash size symbol for linker script
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    -T ${ESP32_LINKER_SCRIPT}
")

# --- Prevent CMake from searching for system tools ---
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER) # Don't look for programs on host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)  # Only look for libs in toolchain root
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)  # Only look for includes in toolchain root
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)  # Only look for packages in toolchain root

# Specify the root for finding libraries, includes, etc. (the toolchain sysroot)
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PATH}/xtensa-esp32-elf)

# Handle CMAKE_INSTALL_PREFIX (usually set by ESP-IDF itself)
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
endif()

