#pragma once
#include <fs/vfs.h>

struct ucfs_superblock {
    char magic[8];
    uint16_t block_size; //in bytes
    uint16_t block_count; //amount of data blocks
    uint16_t block_used; //amount of data blocks used
    uint16_t inode_table_size; //in bytes
    uint32_t block_bitmap[8]; //bitmap of used blocks
};

struct ucfs_inode {
    uint32_t size;
    struct permissions perm;
    time_t mtime;
    uint8_t nlinks;
    uint8_t indirect_block; //indirect block points directly to a dir block if it is a dir
};

struct ucfs_file {
    char name[FS_INAME_LEN + 1];
    uint8_t ino;
};

struct ucfs_filesystem {
    struct filesystem base;
    struct device* dev;
    void* scratch_buffer;
    uint8_t* indirect_buffer;
    uint8_t data_block_offset;
    uint8_t inode_block_offset;
    uint8_t entries_per_dir;
};

extern const struct file_ops ucfs_file_ops;

//file descriptor ops
ssize_t ucfs_read(struct file* f, char* buff, int count);
ssize_t ucfs_write(struct file* f, const char* buff, int count);
int ucfs_readdir(struct file* f, struct dirent* buff, int count);
//off_t (*lseek)(struct file* f, off_t offset, int whence);
//int (*fstat)(struct file* f, struct stat* statbuf);
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int ucfs_mount(struct mount* mountpoint, dev_t devno, int mountflags);
//int (*umount)(struct superblock* fs);
//int (*statfs)(struct superblock* fs);

//struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
//int (*remove)(struct inode* target);

//filesystem lookup function
ino_t ucfs_lookup(struct inode* dir, const char* name); //lookup an inode inside a dir
struct inode* ucfs_read_i(struct filesystem* fs, ino_t ino); //read an inode
//int (*write_i)(struct inode* target); //write an inode



