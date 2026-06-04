#pragma once
#include "types.h"

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

int sysctl(int cmd, void* buffer, ssize_t count);

