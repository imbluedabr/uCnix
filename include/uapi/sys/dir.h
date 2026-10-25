#pragma once
#include "types.h"
#include "../limits.h"

struct dirent {
    ino_t d_ino;
    off_t d_offset;
    char d_name[FS_INAME_LEN + 1];
    uint8_t d_namelen;
};


