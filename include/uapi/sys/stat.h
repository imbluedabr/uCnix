#pragma once
#include "types.h"
#include "../limits.h"


#define S_IFREG     (0b0001 << 12)
#define S_IFDIR     (0b0010 << 12)
#define S_IFDEV     (0b0011 << 12)
#define S_IFMNT     (0b0100 << 12)
#define S_IFSOCK    (0b0101 << 12)

#define S_ISUID     (0b100 << 9)
#define S_ISGID     (0b010 << 9)

#define S_IRUSR     (0b100 << 6)
#define S_IWUSR     (0b010 << 6)
#define S_IXUSR     (0b001 << 6)
#define S_IRGRP     (0b100 << 3)
#define S_IWGRP     (0b010 << 3)
#define S_IXGRP     (0b001 << 3)
#define S_IROTH     (0b100 << 0)
#define S_IWOTH     (0b010 << 0)
#define S_IXOTH     (0b001 << 0)




struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid; 
    dev_t st_rdev; 
    off_t st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

