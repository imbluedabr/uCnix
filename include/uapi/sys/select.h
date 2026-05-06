#pragma once
#include "time.h"

#define FD_SETSIZE 32

#define FD_ZERO(set) \
    memset((set), 0, sizeof(fd_set))

#define FD_SET(fd, set) \
    ((set)->bits[(fd) / (8 * sizeof(unsigned int))] |= \
     (1UL << ((fd) % (8 * sizeof(unsigned int)))))

#define FD_CLR(fd, set) \
    ((set)->bits[(fd) / (8 * sizeof(unsigned int))] &= \
     ~(1UL << ((fd) % (8 * sizeof(unsigned int)))))

#define FD_ISSET(fd, set) \
    ((set)->bits[(fd) / (8 * sizeof(unsigned int))] & \
     (1UL << ((fd) % (8 * sizeof(unsigned int)))))

typedef struct {
    unsigned int bits[FD_SETSIZE / (8*sizeof(unsigned int))];
} fd_set;


