#pragma once
#include <stdint.h>


#define MAJOR(DEVNO) (DEVNO >> 4)
#define MINOR(DEVNO) (DEVNO & 0xF)

#define MKDEV(MAJOR_NO, MINOR_NO) ((MAJOR_NO << 4) | (MINOR_NO & 0xF))

typedef int ino_t; //this is a unique identifier for an inode, this would be like the cluster + offset in fat
typedef uint32_t off_t;
typedef int32_t ssize_t;
typedef uint8_t dev_t;
typedef int pid_t;
typedef uint8_t uid_t;
typedef uint8_t gid_t;
typedef uint8_t nlink_t;
typedef uint16_t mode_t;
typedef uint16_t blksize_t;
typedef uint16_t blkcnt_t;
typedef uint8_t fsid_t;
typedef uint32_t sigset_t;

