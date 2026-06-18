#include <fs/ucfs.h>
#include <lib/kmalloc.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/disk.h>

const struct file_ops ucfs_file_ops = {
    .read = &ucfs_read,
    .write = &ucfs_write,
    .readdir = &ucfs_readdir,
    .fstat = ucfs_fstat,
    .mount = &ucfs_mount,
    .lookup = &ucfs_lookup,
    .read_i = &ucfs_read_i
};

//file descriptor ops
ssize_t ucfs_read(struct file* f, char* buff, int count)
{
    struct inode* i = f->i;
    struct ucfs_filesystem* fs = (struct ucfs_filesystem*) i->fs;
    
    if ((count + f->offset) > i->size) count = i->size - f->offset;

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
    if (fs->indirect_buffer[current_block] == BLOCK_NIL) {
        return 0;
    }
  
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
        device_read(fs->dev, buff + bytes_read, 1, fs->data_block_offset + fs->indirect_buffer[current_block++]);
        bytes_read += 512;
        if (fs->indirect_buffer[current_block] == BLOCK_NIL) {
            return bytes_read;
        }
    }

    if ((count - bytes_read) > 0) {
        int to_read = count - bytes_read;
        device_read(fs->dev, fs->scratch_buffer, 1, fs->data_block_offset + fs->indirect_buffer[current_block++]);
        memcpy(buff + bytes_read, fs->scratch_buffer, to_read);
        bytes_read += to_read;
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
            strlcpy(buff[d_count].d_name, entry->name, FS_INAME_LEN);
            buff[d_count].d_namelen = strnlen(entry->name, FS_INAME_LEN);
            buff[d_count].d_ino = FS_MAKE_UNO(fs->base.fsid, entry->ino);
            d_count++;
        }
        if (d_count >= count) break;
    }

    return d_count;
}

//off_t (*lseek)(struct file* f, off_t offset, int whence);
int ucfs_fstat(struct file* f, struct stat* statbuf)
{
    struct inode* node = f->i;
    struct ucfs_filesystem* ucfs = (struct ucfs_filesystem*) node->fs;
    //calculate the offets of the inode
    int local_ino = FS_GET_INO(node->ino);
    if (local_ino > 255) return -EBADF;
    int address = local_ino*sizeof(struct ucfs_inode);
    int sector = address/ucfs->base.block_size;
    int sector_offset = ucfs->inode_block_offset + sector;
    int index = local_ino - sector*32;
    device_read(ucfs->dev, ucfs->scratch_buffer, 1, sector_offset);

    struct ucfs_inode* i = &((struct ucfs_inode*) ucfs->scratch_buffer)[index];

    statbuf->st_dev = ucfs->base.devno;
    statbuf->st_ino = node->ino;
    statbuf->st_mode = node->perm.mode;
    statbuf->st_nlink = i->nlinks;
    statbuf->st_uid = node->perm.user;
    statbuf->st_gid = node->perm.group;
    statbuf->st_size = node->size;
    statbuf->st_blksize = ucfs->base.block_size;
    statbuf->st_atime = i->mtime;
    statbuf->st_mtime = i->mtime;
    statbuf->st_ctime = i->mtime;

    return 0;
}
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int ucfs_mount(struct mount* mountpoint, dev_t devno, int mountflags)
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
    int e_code;

    ucfs->scratch_buffer = kmalloc(sector_size);
    ucfs->indirect_buffer = kmalloc(256);
    if (!ucfs->scratch_buffer || !ucfs->indirect_buffer) {
        e_code = -ENOMEM;
        goto error;
    }
    if (device_read(dev, ucfs->scratch_buffer, 1, 1) < 0) { //read the superblock
        e_code = -EIO;
        goto error;
    }

    struct ucfs_superblock* sb = ucfs->scratch_buffer;

    kdbg("ucfs: magic=%s\n", sb->magic);
    if (sb->block_size != sector_size) {
        kerr("ucfs: block sizes other then %d are not suported(yet)!\n", sector_size);
        e_code = -EIO;
        goto error;
    }
    
    ucfs->inode_block_offset = 2; //sector 0 is the boot sector, sector 1 is the superblock
    ucfs->data_block_offset = ucfs->inode_block_offset + 4;
    ucfs->base.block_size = sector_size;
    ucfs->base.block_count = sb->block_count;
    ucfs->base.block_used = sb->block_used;
    ucfs->entries_per_dir = sector_size/sizeof(struct file);

    mountpoint->root = ucfs_read_i(&ucfs->base, FS_MAKE_UNO(ucfs->base.fsid, 0));
    kdbg("ucfs: read root inode\n");
    if (!mountpoint->root) {
        e_code = -EIO;
        goto error;
    }
    return 0;
error:
    kfree(ucfs->scratch_buffer);
    kfree(ucfs->indirect_buffer);
    kfree(ucfs);
    return e_code;
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
    if (local_ino > 255) return NULL;
    int address = local_ino*sizeof(struct ucfs_inode);
    int sector = address/ucfs->base.block_size;
    int sector_offset = ucfs->inode_block_offset + sector;
    int index = local_ino - sector*32;
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


