#include <fs/devfs.h>
#include <lib/kmalloc.h>


const struct file_ops devfs_file_ops = {
    .read = &devfs_read,
    .write = devfs_write,
    .readdir = &devfs_readdir,
    .mount = &devfs_mount,
    .lookup = &devfs_lookup,
    .read_i = &devfs_read_i
};

//file descriptor ops
ssize_t devfs_read(struct file* f, char* buff, int count);
ssize_t devfs_write(struct file* f, const char* buff, int count);
int devfs_readdir(struct file* f, struct dirent* buff, int count);
//off_t (*lseek)(struct file* f, off_t offset, int whence);
//int (*fstat)(struct file* f, struct stat* statbuf);
//off_t (*ftruncate)(struct file* f, off_t lenght);

//inode operations
int devfs_mount(struct mount* mountpoint, dev_t devno, int mountflags)
{
    struct devfs_filesystem* devfs = kzalloc(sizeof(struct devfs_filesystem));
    if (!devfs) return -ENOMEM;
    

}
//int (*umount)(struct superblock* fs);
//int (*statfs)(struct superblock* fs);

//struct inode* (*create)(struct inode* dir, const char* name, struct permissions perm);
//int (*remove)(struct inode* target);

//filesystem lookup function
ino_t devfs_lookup(struct inode* dir, const char* name); //lookup an inode inside a dir
struct inode* devfs_read_i(struct filesystem* fs, ino_t ino); //read an inode
//int (*write_i)(struct inode* target); //write an inode



