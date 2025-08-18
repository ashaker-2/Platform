/* ============================================================================
 * SOURCE FILE: HAL/inc/hal_init.h
 * ============================================================================*/
/**
 * @file hal_init.h
 * @brief Public API for initializing the entire Hardware Abstraction Layer.
 * This is the single function the application should call to set up
 * all the necessary hardware peripherals defined within the HAL.
 */
#ifndef HAL_INIT_H
#define HAL_INIT_H

#include "common.h" // For Status_t

/**
 * @brief Initializes all hardware abstraction layer components.
 * This function orchestrates the initialization of GPIO, I2C, ADC, Timer, and SPI
 * peripherals by calling their respective configuration functions.
 * @return E_OK if all HAL components are initialized successfully, otherwise an error code.
 */
Status_t HAL_Init(void);

#endif /* HAL_INIT_H */
