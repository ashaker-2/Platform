\
/* sysmgrux.h - UX SWC (LCD + Keypad) for SysMgr */

#ifndef SYSMGRUX_H
#define SYSMGRUX_H

#include "common.h"
#include <stdint.h>

Status_t SysMgrUX_Init(void);
void SysMgrUX_MainFunction(uint32_t tick_ms);

#endif /* SYSMGRUX_H */
