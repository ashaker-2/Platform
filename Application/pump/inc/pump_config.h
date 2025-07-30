// app/inc/pump_config.h

#ifndef PUMP_CONFIG_H
#define PUMP_CONFIG_H

#include <stdint.h>
#include "pump.h" // Includes PUMP_Config_t definition

// Declare the external constant array defined in pump_config.c
extern const PUMP_Config_t pump_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t PUMP_NUM_CONFIGURATIONS;

#endif /* PUMP_CONFIG_H */
