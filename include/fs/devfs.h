#pragma once
#include <fs/vfs.h>

struct devfs_file {
    char name[FS_INAME_LEN];
    dev_t devno;
    struct permissions perm;
};

struct devfs_filesystem {
    struct filesystem base;
    struct devfs_file files[16];
};











