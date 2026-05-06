#pragma once
#include "types.h"

struct statfs {
    uint16_t f_type;
    uint16_t f_bsize;
    uint16_t f_blocks;
    uint16_t f_bfree;
    uint16_t f_bavail;
    uint16_t f_files;
    uint16_t f_ffree;
    fsid_t f_fsid;
};

