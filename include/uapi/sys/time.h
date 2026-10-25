#pragma once
#include <stdint.h>

#define CLOCK_MONOTONIC 0

typedef uint32_t time_t;
typedef uint64_t suseconds_t;

struct timeval {
    time_t tv_sec; //seconds
    suseconds_t tv_usec; //microseconds
};

struct timespec {
    time_t tv_sec;
    suseconds_t tv_nsec;
};

