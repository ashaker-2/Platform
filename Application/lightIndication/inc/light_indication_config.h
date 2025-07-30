// app/inc/light_indication_config.h

#ifndef LIGHT_INDICATION_CONFIG_H
#define LIGHT_INDICATION_CONFIG_H

#include <stdint.h>
#include "light_indication.h" // Includes LIGHT_INDICATION_Config_t definition

// Declare the external constant array defined in light_indication_config.c
extern const LIGHT_INDICATION_Config_t light_indication_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t LIGHT_INDICATION_NUM_CONFIGURATIONS;

#endif /* LIGHT_INDICATION_CONFIG_H */
