#include <fs/fatfs.h>
#include <kernel/device.h>
#include <kernel/alloc.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <uapi/sys/stat.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/disk.h>

const struct file_ops fatfs_file_ops = {
    .read = &fatfs_read,
    .write = &fatfs_write,
    .readdir = &fatfs_readdir,
    .lseek = &fatfs_lseek,
    .ftruncate = &fatfs_ftruncate,
    .mount = &fatfs_mount,
    .umount = &fatfs_umount,
    .create = &fatfs_create,
    .remove = &fatfs_remove,
    .close = &fatfs_close,
    .lookupn = &fatfs_lookupn
};


static inline ino_t fatfs_calc_ino(struct fatfs_superblock* fs, uint8_t index)
{
    return FS_GET_INO_OFF(fs->base.fsid) + index;
}

//file descriptor ops
ssize_t fatfs_read(struct file* f, char* buff, int count)
{
    struct inode* i = f->i;
    struct fatfs_superblock* fs = (struct fatfs_superblock*) i->fs;
    uint8_t fat_sector = i->fatfs.fat_index;
    
    int sector_offset = f->offset >> FATFS_SECTOR_LEN;
    int byte_offset = f->offset & FATFS_SECTOR_MASK;

    for (int i = 0; i < sector_offset; i++) {
        fat_sector = fs->fat_cache[fat_sector];
        if (fat_sector == FATFS_SECTOR_END) {
            return 0;
        }
    }

    uint32_t bytes_read = 0;

    if (byte_offset != 0) {
        device_read(fs->dev, fs->scratch_buff.raw, fs->blocks_per_sector, (fat_sector + 1)*fs->blocks_per_sector);
        
        int to_read = FATFS_SECTOR_SIZE - byte_offset;
        if (to_read > count) to_read = count;

        memcpy(buff, fs->scratch_buff.raw + byte_offset, to_read);
        bytes_read += to_read;
        fat_sector = fs->fat_cache[fat_sector];
        if (fat_sector == FATFS_SECTOR_END) {
            return bytes_read;
        }
    }

    for (int i = 0; i < (count >> FATFS_SECTOR_LEN); i++) {
        device_read(fs->dev, buff + bytes_read, fs->blocks_per_sector, (fat_sector + 1)*fs->blocks_per_sector);
        bytes_read += FATFS_SECTOR_SIZE;
        fat_sector = fs->fat_cache[fat_sector];
        if (fat_sector == FATFS_SECTOR_END) {
            return bytes_read;
        }
    }

    if ((count - bytes_read) > 0) {
        int to_read = count - bytes_read;
        device_read(fs->dev, fs->scratch_buff.raw, fs->blocks_per_sector, (fat_sector + 1)*fs->blocks_per_sector);

        memcpy(buff + bytes_read, fs->scratch_buff.raw, to_read);
        bytes_read += to_read;
    }

    return bytes_read;
}

ssize_t fatfs_write(struct file* f, const char* buff, int count)
{
    return -ENOSYS;
}

int fatfs_readdir(struct file* f, struct dirent* buff, int count)
{
    struct inode* dir = f->i;
    struct fatfs_superblock* fs = (struct fatfs_superblock*) dir->fs;

    device_read(fs->dev, fs->scratch_buff.raw, fs->blocks_per_sector, (dir->fatfs.fat_index + 1)*fs->blocks_per_sector);
    
    int i = 0;
    for (; i < count; i++) {
        struct dirent* entry = &buff[i];
        struct fatfs_file* fat_entry = &fs->scratch_buff.dirs[i];
        if (fat_entry->fat_index != FATFS_SECTOR_FREE) {
            entry->d_ino = fatfs_calc_ino(fs, fat_entry->fat_index);
            entry->d_offset = i;
            entry->d_namelen = strnlen(fat_entry->name, FS_INAME_LEN);
            strlcpy(entry->d_name, fat_entry->name, FS_INAME_LEN);
        }
        if (i == (FATFS_DIR_LEN - 1)) {
            break;
        }
    }
    return i;
}

int fatfs_fstat(struct file* f, struct stat* statbuff)
{
    return -ENOSYS; //this might be redundant so il remove it later
}

off_t fatfs_lseek(struct file* f, off_t offset, int whence)
{
    return -ENOSYS;
}

off_t fatfs_ftruncate(struct file* f, off_t lenght)
{
    return -ENOSYS;
}

//inode operations
int fatfs_mount(struct inode* mountpoint, dev_t devno, int mountflags) {
    struct fatfs_superblock* fs = kzalloc(sizeof(struct fatfs_superblock));
    if (!fs) {
        return -ENOMEM;
    }
    
    fs->base.fsid = vfs_get_fsid();
    fs->base.fops = (struct file_ops*) &fatfs_file_ops;
    fs->devno = devno;
    fs->dev = device_lookup(devno);
    if (fs->dev == NULL) {
        return -ENODEV;
    }

    mountpoint->fatfs.fat_index = 0;
    mountpoint->perm.mode = S_IFMNT;
    mountpoint->file = fatfs_calc_ino(fs, 0);
    mountpoint->fs = &fs->base;

    uint16_t dev_bsize;
    fs->dev->driver->ioctl(fs->dev, IOCTL_BLK_GETSECSZ, &dev_bsize);

    device_read(fs->dev, fs->scratch_buff.raw, 1, 0); //block 0 contains the disk descriptor
    memcpy(fs->fat_cache, fs->scratch_buff.desc.fat_table, 256);
    
    fs->base.block_count = fs->scratch_buff.desc.block_count;
    fs->base.block_size = fs->scratch_buff.desc.block_size;
    fs->base.block_used = fs->scratch_buff.desc.block_used;
    if (fs->base.block_count == 0 || fs->base.block_used == 0) return -EIO;
    fs->blocks_per_sector = fs->base.block_size/dev_bsize;
    
    return 0;
}

int fatfs_umount(struct superblock* fs)
{
    return -ENOSYS;
}

struct inode* fatfs_create(struct inode* dir, const char* name, struct permissions perm)
{
    return NULL;
}

int fatfs_remove(struct inode* target)
{
    return -ENOSYS;
}

int fatfs_close(struct inode* target)
{
    return -ENOSYS;
}

//filesystem lookup functions
struct inode* fatfs_lookupn(struct inode* dir, const char* name) //lookup an inode in a dir
{
    struct fatfs_superblock* fs = (struct fatfs_superblock*) dir->fs;

    device_read(fs->dev, fs->scratch_buff.raw, fs->blocks_per_sector, (dir->fatfs.fat_index + 1)*fs->blocks_per_sector);

    for (uint32_t i = 0; i < FATFS_DIR_LEN; i++) {
        struct fatfs_file* file = &fs->scratch_buff.dirs[i];
        
        if (strncmp(file->name, name, FS_INAME_LEN) == 0) {
            struct inode* newi = inode_alloc();
            newi->perm = file->perm;
            newi->file = fatfs_calc_ino(fs, file->fat_index);
            newi->dir = dir->file;
            newi->fatfs.fat_index = file->fat_index;
            newi->fs = &fs->base;
            newi->size = file->size;
            strlcpy(newi->name, file->name, FS_INAME_LEN);

            return newi;
        }
    }

    return NULL;
}


