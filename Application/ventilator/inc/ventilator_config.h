// app/inc/ventilator_config.h

#ifndef VENTILATOR_CONFIG_H
#define VENTILATOR_CONFIG_H

#include <stdint.h>
#include "ventilator.h" // Includes VENTILATOR_Config_t definition

// Declare the external constant array defined in ventilator_config.c
extern const VENTILATOR_Config_t ventilator_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t VENTILATOR_NUM_CONFIGURATIONS;

#endif /* VENTILATOR_CONFIG_H */
