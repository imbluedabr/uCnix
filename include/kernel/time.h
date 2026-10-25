#pragma once
#include <stdint.h>
#include <uapi/sys/time.h>

uint32_t get_kernel_ticks();
void simple_delay(uint32_t ticks);
void time_init();


int sys_clock_gettime(int clockid, struct timespec* tp);

