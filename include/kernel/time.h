#pragma once
#include <stdint.h>


uint32_t get_kernel_ticks();
void simple_delay(uint32_t ticks);
void time_init();

