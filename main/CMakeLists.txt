set(APP_DIR     "${CMAKE_CURRENT_LIST_DIR}/../Application")
set(HAL_DIR     "${CMAKE_CURRENT_LIST_DIR}/../HAL")
set(Rte_DIR     "${CMAKE_CURRENT_LIST_DIR}/../Rte")
set(Service_DIR "${CMAKE_CURRENT_LIST_DIR}/../Service")


# Load source and include lists
include(${APP_DIR}/cmake/application_file_lists.cmake)
include(${HAL_DIR}/cmake/hal_file_lists.cmake)
include(${Rte_DIR}/cmake/rte_file_lists.cmake)
include(${Service_DIR}/cmake/Service_file_lists.cmake)


idf_component_register(
    SRCS
        ${APPLICATION_SRCS}
        # ${HAL_SRCS}
    INCLUDE_DIRS
        ${APP_DIR}/common/inc
        ${APPLICATION_PUBLIC_INC_DIRS}
        ${HAL_PUBLIC_INC_DIRS}
        ${RTE_PUBLIC_INC_DIRS}
        ${SERVICE_PUBLIC_INC_DIRS}
        
    REQUIRES
        driver              # <-- this is what gives you <driver/gpio.h>
        freertos            # 🔹 ADD THIS for <freertos/FreeRTOS.h>
        ${APPLICATION_PUBLIC_REQUIRES}
    PRIV_REQUIRES
        ${APPLICATION_PRIVATE_REQUIRES}
)

