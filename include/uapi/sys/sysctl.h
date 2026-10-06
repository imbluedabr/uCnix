#pragma once
#include "types.h"
#include <stddef.h>

struct pstat {
    off_t program_base;
    ssize_t program_size;
    uint8_t state;
    pid_t pid;
    pid_t ppid;
    pid_t pgrp;
    uid_t ruid;
    gid_t rgid;
};

struct mstat {
    size_t blocks_used;
    size_t blocks_total;
    int fragmentation;
    size_t bytes_used;
    size_t bytes_total;
};

int sysctl(int cmd, void* buffer, ssize_t count);

