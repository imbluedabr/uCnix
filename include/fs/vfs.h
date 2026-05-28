#pragma once

#include <uapi/sys/types.h>
#include <uapi/sys/stat.h>
#include <uapi/sys/dir.h>
#include <uapi/sys/select.h>
#include <uapi/sys/statfs.h>
#include <uapi/limits.h>
#include <kernel/device.h>
#include <kernel/lock.h>
#include <kernel/settings.h>
#include <stdint.h>

struct superblock;
struct file;
struct dentry;
struct file_ops;

//make a unique number
#define FS_MAKE_UNO(ID, NO) ((ID << 24) + NO)
//retrieve inode number
#define FS_GET_INO(INO) (INO & 0xFFFFFF)

#define VFS_LOOKUP_BASE 1

struct permissions {
    uid_t user;
    gid_t group;
    mode_t mode;
};

struct inode {
    struct filesystem* fs;
    struct inode* next;
    ino_t ino;
    struct permissions perm;
    uint32_t size : 24;
    uint32_t refcount : 8;
    union {
        struct { //filesystem specific metadata
            dev_t devno;
        } devfs;
        struct {
            uint8_t indirect_block;
        } ucfs;
    };
};

struct dentry {
    char name[FS_INAME_LEN + 2];
    ino_t parent_ino;
    ino_t ino;
};

struct mount {
    struct inode* mountpoint;
    struct inode* root;
};

struct filesystem {
    const struct file_ops* fops;
    uint8_t fsid; //filesystem id, used by filesystems to ensure unique inode numbers
    dev_t devno;
    uint16_t block_size;
    uint16_t block_count;
    uint16_t block_used;
};

struct file {
    struct inode* i;
    uint32_t offset : 24;
    uint8_t refcount : 4;
    uint8_t flags : 4;
};

//file operations
struct file_ops {
    //file descriptor ops
    ssize_t (*read)(struct file* f, char* buff, int count);
    ssize_t (*write)(struct file* f, const char* buff, int count);
    int (*readdir)(struct file* f, struct dirent* buff, int count);
    off_t (*lseek)(struct file* f, off_t offset, int whence);
    int (*fstat)(struct file* f, struct stat* statbuf);
    off_t (*ftruncate)(struct file* f, off_t lenght);

    //filesystem operations
    int (*mount)(struct mount* mountpoint, dev_t devno, int mountflags);
    int (*umount)(struct filesystem* fs);
    int (*statfs)(struct filesystem* fs);
    
    //inode operations
    struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
    int (*link)(struct inode* dir, struct inode* target, const char* name);
    int (*unlink)(struct inode* dir, const char* name);
    int (*close)(struct inode* targer);

    //filesystem lookup function
    ino_t (*lookup)(struct inode* dir, const char* name); //lookup an inode inside a dir
    struct inode* (*read_i)(struct filesystem* fs, ino_t ino); //read an inode
    int (*write_i)(struct inode* target); //write an inode
};



extern mutex_t vfs_cache_lock;
struct inode* inode_alloc();
void inode_free(struct inode* i);
void file_free(struct file* f);

extern struct file vfs_file_table[VFS_MAXFILES];
extern struct mount mount_table[VFS_MAXMOUNTS];

void vfs_init();
int vfs_get_fsid();
struct inode* vfs_walk_path(const char* path, int mode);
struct file_ops* get_filesystem(const char* name);
int vfs_mount_root(dev_t devno, const char* filesystemtype, int mountflags);


ssize_t vfs_read(int fd, void* buffer, size_t count);
ssize_t vfs_write(int fd, const void* buffer, size_t count);
int vfs_open(const char* path, int flags);
int vfs_close(int fd);
int vfs_fstat(int fd, struct stat *statbuf);
off_t vfs_lseek(int fd, off_t offset, int whence);
int vfs_ioctl(int fd, int cmd, void* arg);
int vfs_access(const char* path, int mode);
int vfs_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
int vfs_fcntl(int fd, int op, int arg);
int vfs_flock(int fd, int op);
int vfs_fsync(int fd);
int vfs_ftruncate(int fd, off_t lenght);
int vfs_chdir(const char* path);
ssize_t vfs_readdir(int fd, struct dirent* buf, size_t count);
int vfs_mkdir(const char* path, mode_t mode);
int vfs_rmdir(const char* path);
int vfs_rename(const char* oldpath, const char* newpath);
int vfs_unlink(const char* path);
int vfs_symlink(const char* target, const char* linkpath);
int vfs_readlink(const char* path, char* buf, size_t bufsiz);
int vfs_fstatfs(int fd, struct statfs* buf);
int vfs_fchmod(int fd, mode_t mode);
int vfs_fchown(int fd, uid_t owner, gid_t group);
int vfs_sync();
int vfs_mount(const char* source, const char* target, const char* filesystemtype, int mountflags);
int vfs_umount(const char* target);
int vfs_statfs(const char* path, struct statfs* buff);
