#pragma once
#include <uapi/sys/types.h>
#include <uapi/limits.h>
#include <fs/vfs.h>

struct fatfs_file {
    char name[FS_INAME_LEN + 1];
    uint8_t fat_index;
    struct permissions perm;
    time_t time;
    uint16_t size;
};


#define FATFS_SECTOR_LEN 9
#define FATFS_SECTOR_SIZE (1 << FATFS_SECTOR_LEN)
#define FATFS_SECTOR_MASK (FATFS_SECTOR_SIZE - 1)
#define FATFS_DIR_LEN (FATFS_SECTOR_SIZE/sizeof(struct fatfs_file))
#define FATFS_FAT_START 0
#define FATFS_DATA_START 512

#define CALC_FADRES(INDEX) (INDEX + FATFS_FAT_START)
#define CALC_SADRES(INDEX) (INDEX*FATFS_SECTOR_SIZE + FATFS_DATA_START)

#define FATFS_SECTOR_END 0xFF
#define FATFS_SECTOR_FREE 0xFE

struct fatfs_superblock {
    struct superblock base;
    uint8_t fat_cache[256];
    union {
        struct fatfs_file dirs[FATFS_DIR_LEN];
        uint8_t raw[FATFS_SECTOR_SIZE];
    } scratch_buff;
    dev_t devno;
    struct device* dev;
};

extern const struct file_ops fatfs_file_ops;

//file descriptor ops
ssize_t fatfs_read(struct file* f, char* buff, int count);
ssize_t fatfs_write(struct file* f, const char* buff, int count);
int fatfs_readdir(struct file* f, struct dirent* buff, int count);
off_t fatfs_lseek(struct file* f, off_t offset, int whence);
off_t fatfs_ftruncate(struct file* f, off_t lenght);

//inode operations
int fatfs_mount(struct inode* mountpoint, dev_t devno, const char* args);
int fatfs_umount(struct superblock* fs);

struct inode* fatfs_create(struct inode* dir, const char* name, struct permissions perm);
int fatfs_remove(struct inode* target);
int fatfs_close(struct inode* target);

int fatfs_setperm(struct inode* target, struct permissions perm);

//filesystem lookup functions
struct inode* fatfs_lookupn(struct inode* dir, const char* name); //lookup an inode in a dir

