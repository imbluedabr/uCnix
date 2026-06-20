#pragma once
#include <fs/vfs.h>

struct devfs_file {
    char name[FS_INAME_LEN + 1];
    dev_t devno;
    struct permissions perm;
};

struct devfs_filesystem {
    struct filesystem base;
    struct devfs_file files[16];
};

extern const struct file_ops devfs_file_ops;

//file descriptor ops
ssize_t devfs_read(struct file* f, char* buff, int count);
ssize_t devfs_write(struct file* f, const char* buff, int count);
int devfs_readdir(struct file* f, struct dirent* buff, int count);
//off_t (*lseek)(struct file* f, off_t offset, int whence);
int devfs_fstat(struct file* f, struct stat* statbuf);
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int devfs_mount(struct mount* mountpoint, dev_t devno, int mountflags);
//int (*umount)(struct superblock* fs);
//int (*statfs)(struct superblock* fs);
int devfs_mknod(struct filesystem* fs, const char* name, struct permissions perm, dev_t devno);

//struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
//int (*remove)(struct inode* target);

//filesystem lookup function
ino_t devfs_lookup(struct inode* dir, const char* name); //lookup an inode inside a dir
struct inode* devfs_read_i(struct filesystem* fs, ino_t ino); //read an inode
//int (*write_i)(struct inode* target); //write an inode












