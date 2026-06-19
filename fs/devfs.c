#include <fs/devfs.h>
#include <lib/kmalloc.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <uapi/sys/errno.h>


const struct file_ops devfs_file_ops = {
    .read = &devfs_read,
    .write = devfs_write,
    .readdir = &devfs_readdir,
    .mount = &devfs_mount,
    .mknod = &devfs_mknod,
    .lookup = &devfs_lookup,
    .read_i = &devfs_read_i
};

//file descriptor ops
ssize_t devfs_read(struct file* f, char* buff, int count)
{
    struct inode* i = f->i;
    if (i->devfs.dev && i->perm.mode & S_IFDEV) {
        return device_read(i->devfs.dev, buff, count, f->offset);
    }
    return -EIO;
}

ssize_t devfs_write(struct file* f, const char* buff, int count)
{
    struct inode* i = f->i;
    if (i->devfs.dev && i->perm.mode & S_IFDEV) {
        return device_write(i->devfs.dev, (void*) buff, count, f->offset);
    }
    return -EIO;
}

static inline void mkden(struct dirent* buff, int d_count, ino_t ino, const char* name) {
    struct dirent* d = &buff[d_count];
    d->d_ino = ino;
    d->d_namelen = strnlen(name, FS_INAME_LEN);
    d->d_offset = d_count;
    strlcpy(d->d_name, name, FS_INAME_LEN);
}

int devfs_readdir(struct file* f, struct dirent* buff, int count)
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) f->i->fs;
    
    int curr_offset = 2;
    int offset = f->offset;
    int d_count = 0;

    if (offset == 1 && count > 0) {
        mkden(buff, d_count++, FS_MAKE_UNO(devfs->base.fsid, 255), "..");
        if (d_count == count) return d_count;
    }

    for (int i = 0; i < 16; i++) {
        struct devfs_file* d = &devfs->files[i];
        if (d->devno != 255) {
            if (curr_offset >= offset) {
                mkden(buff, d_count++, FS_MAKE_UNO(devfs->base.fsid, i), d->name);
            }
            curr_offset++;
        }
        if (d_count >= count) break;
    }

    return d_count;
}

//off_t (*lseek)(struct file* f, off_t offset, int whence);
//int (*fstat)(struct file* f, struct stat* statbuf);
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int devfs_mount(struct mount* mountpoint, dev_t devno, int mountflags)
{
    struct devfs_filesystem* devfs = kzalloc(sizeof(struct devfs_filesystem));
    if (!devfs) return -ENOMEM;
    
    devfs->base.fops = &devfs_file_ops;
    devfs->base.fsid = vfs_get_fsid();

    for (int i = 0; i < 16; i++) {
        devfs->files[i].devno = 255;
    }
    kdbg("devfs: creating root inode\n");
    mountpoint->root = devfs_read_i(&devfs->base, FS_MAKE_UNO(devfs->base.fsid, 255));
    if (mountpoint->root == NULL) {
        kfree(devfs);
        return -EIO;
    }

    return 0;
}
//int (*umount)(struct superblock* fs);
//int (*statfs)(struct superblock* fs);
int devfs_mknod(struct filesystem* fs, const char* name, struct permissions perm, dev_t devno)
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) fs;
    FS_SET_FTYPE(perm, S_IFDEV);
    for (int i = 0; i < 16; i++) {
        struct devfs_file* f = &devfs->files[i];
        if (f->devno == 255) {
            kinfo("devfs: creating handle (%s) with acces mode 0x%x\n", name, perm.mode);
            f->perm = perm;
            f->devno = devno;
            strlcpy(f->name, name, FS_INAME_LEN);
            return 0;
        }
    }
    return -ENOSPC;
}



//struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
//int (*remove)(struct inode* target);

//filesystem lookup function
ino_t devfs_lookup(struct inode* dir, const char* name)
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) dir->fs;
    
    if (strncmp(name, "..", FS_INAME_LEN) == 0) {
        return FS_MAKE_UNO(devfs->base.fsid, 255);
    }

    for (int i = 0; i < 16; i++) {
        struct devfs_file* f = &devfs->files[i];
        if (strncmp(f->name, name, FS_INAME_LEN) == 0 && f->devno != 255) {
            return FS_MAKE_UNO(devfs->base.fsid, i);
        }
    }
    return -ENOENT;
}

struct inode* devfs_read_i(struct filesystem* fs, ino_t ino) //read an inode
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) fs;
    
    uint32_t index = FS_GET_INO(ino);
    struct inode* newi = inode_alloc();
    if (!newi) return NULL;
    newi->fs = fs;
    newi->ino = ino;
    newi->size = 0;
    
    if (index == 255) {
        newi->perm.mode = 020755;
    } else if (index < 16) {
        struct devfs_file* f = &devfs->files[index];
        if (f->devno == 255) goto error;
        newi->perm = f->perm;
        newi->devfs.dev = device_lookup(f->devno);
    } else {
        goto error;
    }

    return newi;

error:
    inode_free(newi);
    return NULL;
}

//int (*write_i)(struct inode* target); //write an inode



