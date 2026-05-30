#pragma once
#include "time.h"

#define FD_SETSIZE 32

typedef struct {
    unsigned int bits;
} fd_set;

#define FD_ZERO(SET) memset(&SET, 0, sizeof(fd_set))
#define FD_SET(FD, SET) (SET.bits |= (1 << FD))
#define FD_CLR(FD, SET) (SET.bits &= ~(1 << FD))
#define FD_ISSET(FD, SET) (SET.bits & (1 << FD))

