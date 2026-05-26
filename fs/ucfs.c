#include <fs/ucfs.h>
#include <kernel/alloc.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/disk.h>

const struct file_ops ucfs_file_ops = {
    .read = &ucfs_read,
    .write = &ucfs_write,
    .readdir = &ucfs_readdir,
    .mount = &ucfs_mount,
    .lookup = &ucfs_lookup,
    .read_i = &ucfs_read_i
};

//file descriptor ops
ssize_t ucfs_read(struct file* f, char* buff, int count)
{
    struct inode* i = f->i;
    struct ucfs_filesystem* fs = (struct ucfs_filesystem*) i->fs;
    //read the indirect block
    int n_read = device_read(fs->dev, fs->scratch_buffer, 1, fs->data_block_offset + i->ucfs.indirect_block);    
    if (n_read < 0) return n_read;
    //i use 8 bit block indexes, for a maximum of 256 blocks, e.g 128kb using 512 byte blocks
    memcpy(fs->indirect_buffer, fs->scratch_buffer, 256);
    
    //just hardcoding everything for now, dont have the time or energy to allow for a variabe block size and variable sector size
    uint8_t block_offset = f->offset >> 9;
    uint16_t byte_offset = f->offset & 0x1FF;
    
    uint32_t bytes_read = 0;
    uint8_t current_block = block_offset;

    if (byte_offset != 0) {
        device_read(fs->dev, fs->scratch_buffer, 1, fs->data_block_offset + fs->indirect_buffer[current_block]);
        current_block++;
        int to_read = 512 - byte_offset;
        if (to_read > count) to_read = count;
        
        memcpy(buff, ((uint8_t*)fs->scratch_buffer) + byte_offset, to_read);
        bytes_read += to_read;
        if (fs->indirect_buffer[current_block] == BLOCK_NIL) {
            return bytes_read;
        }
    }
    
    
    for (int i = 0; i < (count >> 9); i++) {
        device_read(fs->dev, buff, 1, fs->data_block_offset + fs->indirect_buffer[current_block++]);
        bytes_read += 512;
        if (fs->indirect_buffer[current_block] == BLOCK_NIL) {
            return bytes_read;
        }
    }

    if ((count - bytes_read) > 0) {
        int to_read = count - bytes_read;
        device_read(fs->dev, fs->scratch_buffer, 1, fs->indirect_buffer[current_block++]);
        memcpy(buff + bytes_read, fs->scratch_buffer, to_read);
    }

    return bytes_read;
}

ssize_t ucfs_write(struct file* f, const char* buff, int count)
{
    return -ENOSYS;
}

int ucfs_readdir(struct file* f, struct dirent* buff, int count)
{
    struct inode* i = f->i;
    struct ucfs_filesystem* fs = (struct ucfs_filesystem*) i->fs;
    //read the block of the directory, i use the indirect block pointer as a direct pointer here
    int n_read = device_read(fs->dev, fs->scratch_buffer, 1, fs->data_block_offset + i->ucfs.indirect_block);
    if (n_read < 0) return n_read;

    struct ucfs_file* dir = fs->scratch_buffer;
    int d_count = 0;
    for (int i = 0; i < fs->entries_per_dir; i++) {
        struct ucfs_file* entry = &dir[i];
        if (entry->ino != 255) {
            strlcpy(buff[i].d_name, entry->name, FS_INAME_LEN);
            buff[i].d_namelen = strnlen(entry->name, FS_INAME_LEN);
            buff[i].d_ino = FS_MAKE_UNO(fs->base.fsid, entry->ino);
            d_count++;
        }
    }

    return d_count;
}

//off_t (*lseek)(struct file* f, off_t offset, int whence);
//int (*fstat)(struct file* f, struct stat* statbuf);
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int ucfs_mount(struct inode* mountpoint, dev_t devno, int mountflags)
{
    struct ucfs_filesystem* ucfs = kzalloc(sizeof(struct ucfs_filesystem));
    if (!ucfs) {
        kfree(ucfs);
        return -ENOMEM;
    }

    struct device* dev = device_lookup(devno);
    if (!dev) {
        kfree(ucfs);
        return -ENODEV;
    }


    ucfs->base.fops = &ucfs_file_ops;
    ucfs->base.fsid = vfs_get_fsid();
    ucfs->base.devno = devno;
    ucfs->dev = dev;
    size_t sector_size;
    int status = dev->driver->ioctl(dev, IOCTL_BLK_GETSECSZ, &sector_size);
    if (status < 0) {
        kfree(ucfs);
        return -ENODEV;
    }

    ucfs->scratch_buffer = kmalloc(sector_size);
    if (device_read(dev, ucfs->scratch_buffer, 1, 1) < 0) { //read the superblock
        kfree(ucfs->scratch_buffer);
        kfree(ucfs);
        return -EIO;
    }

    struct ucfs_superblock* sb = ucfs->scratch_buffer;

    kdbg("ucfs: magic=%s\n", sb->magic);
    if (sb->block_size != sector_size) {
        kfree(ucfs->scratch_buffer);
        kfree(ucfs);
        kerr("ucfs: block sizes other then %d are not suported(yet)!\n", sector_size);
        return -EIO;
    }
    
    ucfs->inode_block_offset = 2; //sector 0 is the boot sector, sector 1 is the superblock
    ucfs->data_block_offset = ucfs->inode_block_offset + 4;
    ucfs->base.block_size = sector_size;
    ucfs->base.block_count = sb->block_count;
    ucfs->base.block_used = sb->block_used;
    ucfs->entries_per_dir = sector_size/sizeof(struct file);

    mountpoint->fs = &ucfs->base;
    mountpoint->ino = 0;
    mountpoint->ucfs.indirect_block = 0;
    return 0;
}

//int (*umount)(struct superblock* fs);
//int (*statfs)(struct superblock* fs);

//struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
//int (*remove)(struct inode* target);

//filesystem lookup function
ino_t ucfs_lookup(struct inode* dir, const char* name)
{
    struct ucfs_filesystem* fs = (struct ucfs_filesystem*) dir->fs;
    //read the directory block
    int n_read = device_read(fs->dev, fs->scratch_buffer, 1, fs->data_block_offset + dir->ucfs.indirect_block);

    if (n_read < 0) return n_read;
    
    struct ucfs_file* entries = fs->scratch_buffer;

    for (int i = 0; i < fs->entries_per_dir; i++) {
        struct ucfs_file* entry = entries + i;
        if (strncmp(entry->name, name, FS_INAME_LEN) == 0) {
            return FS_MAKE_UNO(fs->base.fsid, entry->ino);
        }
    }
    return -ENOENT;
}

struct inode* ucfs_read_i(struct filesystem* fs, ino_t ino)
{
    struct ucfs_filesystem* ucfs = (struct ucfs_filesystem*) fs;
    //calculate the offets of the inode
    int local_ino = FS_GET_INO(ino);
    int address = local_ino*sizeof(struct ucfs_inode);
    int sector = address/ucfs->base.block_size;
    int sector_offset = ucfs->inode_block_offset + sector;
    int index = local_ino - sector*32;
    //kdbg("lba = %d\n", sector_offset);
    kprintf("%d\n", sector_offset);
    device_read(ucfs->dev, ucfs->scratch_buffer, 1, sector_offset);

    struct ucfs_inode* i = &((struct ucfs_inode*) ucfs->scratch_buffer)[index];

    struct inode* n = inode_alloc();
    if (!n) return NULL;
    mutex_lock(&vfs_cache_lock);
    n->fs = fs;
    n->ino = ino;
    n->perm = i->perm;
    n->size = i->size;
    n->ucfs.indirect_block = i->indirect_block;
    mutex_unlock(&vfs_cache_lock);
    return n;
}

//int (*write_i)(struct inode* target); //write an inode


