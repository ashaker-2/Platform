# cmake/global_settings.cmake
#
# This file defines global build settings for the entire project.
# It is included by the top-level CMakeLists.txt and individual component
# option files to provide a consistent baseline.

if(NOT DEFINED APP_DIR)
    # Convert source directory to the canonical form
    file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" CURRENT_SRC_DIR)
endif()

# Set global paths using CACHE, ensuring they are only set once
if(NOT DEFINED APP_DIR)
  set(APP_DIR "${CURRENT_SRC_DIR}/Application" CACHE PATH "Path to Application directory")
endif()

if(NOT DEFINED FACTORY_DIR)
  set(FACTORY_DIR "${CURRENT_SRC_DIR}/Factory" CACHE PATH "Path to Factory directory")
endif()

if(NOT DEFINED HAL_DIR)
  set(HAL_DIR "${CURRENT_SRC_DIR}/HAL" CACHE PATH "Path to HAL directory")
endif()

if(NOT DEFINED MCAL_DIR)
  set(MCAL_DIR "${CURRENT_SRC_DIR}/Mcal" CACHE PATH "Path to MCAL directory")
endif()

if(NOT DEFINED SERVICE_DIR)
  set(SERVICE_DIR "${CURRENT_SRC_DIR}/Service" CACHE PATH "Path to Service directory")
endif()

if(NOT DEFINED RTE_DIR)
  set(RTE_DIR "${CURRENT_SRC_DIR}/Rte" CACHE PATH "Path to RTE directory")
endif()

if(NOT DEFINED THIRDPARTY_DIR)
  set(THIRDPARTY_DIR "${CURRENT_SRC_DIR}/Thirdparty" CACHE PATH "Path to Thirdparty directory")
endif()

if(NOT DEFINED APP_DIR)
    # Print out for verification
    message(STATUS "APP_DIR: ${APP_DIR}")
    message(STATUS "FACTORY_DIR: ${FACTORY_DIR}")
    message(STATUS "HAL_DIR: ${HAL_DIR}")
    message(STATUS "MCAL_DIR: ${MCAL_DIR}")
    message(STATUS "SERVICE_DIR: ${SERVICE_DIR}")
    message(STATUS "RTE_DIR: ${RTE_DIR}")
    message(STATUS "THIRDPARTY_DIR: ${THIRDPARTY_DIR}")
endif()

# Define global C compiler options
set(GLOBAL_C_COMPILE_OPTIONS
    -Os                   # Optimize for size
    -Wall                 # Enable all warnings
    -Wextra               # Enable extra warnings
    -fno-builtin          # Avoid built-in functions
    -ffunction-sections   # Place each function in its own section
    -fdata-sections       # Place each data item in its own section
    # Add any other universally applied C flags here
)

# Define global C++ compiler options (if you have C++ in your project)
set(GLOBAL_CXX_COMPILE_OPTIONS
    ${GLOBAL_C_COMPILE_OPTIONS} # Inherit C flags
    -fno-exceptions       # Disable exceptions for embedded
    -fno-rtti             # Disable RTTI for embedded
    # Add any other universally applied C++ flags here
)

# Define global compile definitions (macros applied to all source files)
set(GLOBAL_COMPILE_DEFINITIONS
    -DPROJECT_VERSION="1.0.0"   # Example: Global project version
    -DLOG_LEVEL_INFO            # Example: Default logging level
    -DCONFIG_SOME_FEATURE_ENABLED # Example: A globally enabled feature
    -DXT_BOARD                    # Example: Build for HW
    # -DXT_SIMULATOR                    # Example: Build for Simualtor

)

# Define global include directories that all components might need
# This might include paths to common utility headers or SDK headers
# (Note: For system-level headers, the toolchain/SDK setup often handles this.)
set(GLOBAL_SERVICE_INCLUDE_DIRS

  "${SERVICE_DIR}/OS/FreeRTOS-Kernel-SMP/include/freertos" 
  "${SERVICE_DIR}/OS/FreeRTOS-Kernel-SMP/portable/xtensa/include/freertos" 
  "${SERVICE_DIR}/OS/config/include/freertos" 
  "${THIRDPARTY_DIR}/newlib/platform_include"
  "${THIRDPARTY_DIR}/soc/esp32/register"
  "${THIRDPARTY_DIR}/esp_system/include"
  "${THIRDPARTY_DIR}/soc/include"
  "${THIRDPARTY_DIR}/soc/esp32/include"
  "${THIRDPARTY_DIR}/xtensa/include"
  "${THIRDPARTY_DIR}/xtensa/esp32/include"
  "${THIRDPARTY_DIR}/xtensa/deprecated_include"
  "${THIRDPARTY_DIR}/esp_common/include"
  "${THIRDPARTY_DIR}/esp_hw_support/include"
  "${THIRDPARTY_DIR}/esp_rom/include"
  "${THIRDPARTY_DIR}/log/include"
  "${THIRDPARTY_DIR}/heap/include"    
    
)


set(GLOBAL_APP_INCLUDE_DIRS
    
  "${APP_DIR}/temperature/inc"    
  "${APP_DIR}/fan/inc"            
  "${APP_DIR}/heater/inc"         
  "${APP_DIR}/humadity/inc"       
  # "${APP_DIR}/display/inc"        
  # "${APP_DIR}/diagnostic/inc"     
  "${APP_DIR}/lightControl/inc"   
  "${APP_DIR}/lightIndication/inc"
  "${APP_DIR}/logger/inc"         
  "${APP_DIR}/OTA/inc"            
  "${APP_DIR}/power/inc"          
  "${APP_DIR}/pump/inc"           
  "${APP_DIR}/systemMgr/inc"      
  "${APP_DIR}/SystemMonitor/inc"  
  "${APP_DIR}/SystemStartup/inc"  
  "${APP_DIR}/ventilator/inc"     
           
)

set(GLOBAL_HAL_INCLUDE_DIRS
    
  "${HAL_DIR}/inc" 
 
)
set(GLOBAL_COMMON_INCLUDE_DIRS
    
  "${APP_DIR}/common/inc"
 
)

set(GLOBAL_RTE_INCLUDE_DIRS

  "${RTE_DIR}/inc" 
)

# You can also set global linker options here if they apply to the entire final executable.
# set(GLOBAL_LINK_OPTIONS "-Wl,--gc-sections")

# These variables will be available to any CMakeLists.txt or .cmake file
# that includes this file.