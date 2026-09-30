#pragma once

#include "types.h"

#define PIT_FREQUENCY 1193182
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

void   pit_init(uint32 Frequency);
uint32 pit_get_ticks(void);
void   pit_handler(void);
void   pit_sleep(uint32 Milliseconds);
