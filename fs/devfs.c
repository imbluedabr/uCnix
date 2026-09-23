#include "fs/vfs.h"
#include "kernel/device.h"
#include <fs/devfs.h>
#include <lib/kmalloc.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <uapi/sys/errno.h>


const struct file_ops devfs_file_ops = {
    .read = &devfs_read,
    .write = devfs_write,
    .readdir = &devfs_readdir,
    .fstat = &devfs_fstat,
    .mount = &devfs_mount,
    .mknod = &devfs_mknod,
    .lookup = &devfs_lookup,
    .read_i = &devfs_read_i
};

//file descriptor ops
ssize_t devfs_read(struct file* f, char* buff, int count)
{
    struct inode* i = f->i;
	struct device* dev = i->devfs.dev;
    if (dev && FS_GET_FTYPE(i->perm) == S_IFDEV) {
        return dev->ops->read(f, buff, count);
    }
    return -EIO;
}

ssize_t devfs_write(struct file* f, const char* buff, int count)
{
    struct inode* i = f->i;
	struct device* dev = i->devfs.dev;
    if (dev && FS_GET_FTYPE(i->perm) == S_IFDEV) {
        return dev->ops->write(f, (void*) buff, count);
    }
    return -EIO;
}

static inline void mkden(struct dirent* buff, int d_count, ino_t ino, const char* name) {
    struct dirent* d = &buff[d_count];
    d->d_ino = ino;
    d->d_namelen = strnlen(name, FS_INAME_LEN);
    d->d_offset = d_count;
    strlcpy(d->d_name, name, 9);
}

int devfs_readdir(struct file* f, struct dirent* buff, int count)
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) f->i->fs;
    
    int curr_offset = 2;
    int offset = f->offset;
    int d_count = 0;

    if (offset == 1 && count > 0) {
        mkden(buff, d_count++, FS_MAKE_UNO(devfs->base.fsid, MKDEV(255, 0)), "..");
        if (d_count == count) return d_count;
    }

    for (int i = 0; i < 16; i++) {
        struct devfs_file* d = &devfs->files[i];
        if (d->devno != MKDEV(255, 255)) {
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
int devfs_fstat(struct file* f, struct stat* statbuf)
{
    struct inode* node = f->i;
    //struct devfs_filesystem* devfs = (struct devfs_filesystem*) node->fs;
    
	statbuf->st_rdev = FS_GET_INO(node->ino);
   
    statbuf->st_dev = 0;
    statbuf->st_ino = node->ino;
    statbuf->st_mode = node->perm.mode;
    statbuf->st_nlink = 1;
    statbuf->st_uid = node->perm.user;
    statbuf->st_gid = node->perm.group;
    statbuf->st_size = node->size;
    statbuf->st_blksize = 0;
    statbuf->st_atime = 0;
    statbuf->st_mtime = 0;
    statbuf->st_ctime = 0;

    return 0;
}
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int devfs_mount(struct mount* mountpoint, dev_t devno, int mountflags)
{
    struct devfs_filesystem* devfs = kzalloc(sizeof(struct devfs_filesystem));
    if (!devfs) return -ENOMEM;
    
    devfs->base.fops = &devfs_file_ops;
    devfs->base.fsid = vfs_get_fsid();

    for (int i = 0; i < 16; i++) {
        devfs->files[i].devno = MKDEV(255, 255);
    }
    kdbg("devfs: creating root inode\n");
    mountpoint->root = devfs_read_i(&devfs->base, FS_MAKE_UNO(devfs->base.fsid, MKDEV(255, 0)));
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
        if (f->devno == MKDEV(255,255)) {
            kinfo("devfs: creating handle (%s) with acces mode 0%o\n", name, perm.mode);
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
        return FS_MAKE_UNO(devfs->base.fsid, MKDEV(255, 0));
    }

    for (int i = 0; i < 16; i++) {
        struct devfs_file* f = &devfs->files[i];
        if (strncmp(f->name, name, 10) == 0 && f->devno != MKDEV(255, 255)) {
            return FS_MAKE_UNO(devfs->base.fsid, f->devno);
        }
    }
    return -ENOENT;
}

static struct devfs_file* get_file(struct devfs_filesystem* fs, dev_t devno)
{
	for (int i = 0; i < 16; i++) {
		struct devfs_file* f = &fs->files[i];
		if (f->devno == devno) return f;
	}
	return NULL;
}

struct inode* devfs_read_i(struct filesystem* fs, ino_t ino) //read an inode
{
    struct devfs_filesystem* devfs = (struct devfs_filesystem*) fs;
    
    uint32_t devno = FS_GET_INO(ino);
    struct inode* newi = inode_alloc();
    if (!newi) return NULL;
    newi->fs = fs;
    newi->ino = ino;
    newi->size = 0;

	struct device* dev = device_lookup(devno);
    
    if (devno == MKDEV(255, 0)) {
        newi->perm.mode = 020755;
        newi->size = sizeof(devfs->files);
    } else if (dev) {
        struct devfs_file* f = get_file(devfs, devno);
        if (f->devno == MKDEV(255, 255)) goto error;
        newi->perm = f->perm;
        newi->devfs.dev = dev;
    } else {
        goto error;
    }

    return newi;

error:
    inode_free(newi);
    return NULL;
}

//int (*write_i)(struct inode* target); //write an inode

