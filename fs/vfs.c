#include <fs/vfs.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/fcntl.h>
#include <fs/fatfs.h>
#include <kernel/alloc.h>
#include <kernel/interrupt.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/stdlib.h>

static struct inode* free_list;
static struct inode* cache_list;
static uint8_t cache_list_size;
static uint8_t free_list_size;
mutex_t vfs_cache_lock;

#define CACHE_FLUSH_THRESHOLD 16

/* The inode cache works by first keeping a free list of free inodes. Then when an inode is allocated its removed from the free list and added to the cache list.
 * When an inodes refcount reaches 0 we keep it inside the cache list so its cached, then when the cache gets too big we flush it and only then will the inodes return to the original free list.
*/

struct inode* inode_alloc()
{
    mutex_lock(&vfs_cache_lock);
    if (!free_list) {
        return kmalloc(sizeof(struct inode));
    }

    struct inode* i = free_list;
    free_list = i->next;
    i->next = cache_list;
    cache_list = i;
    free_list_size--;
    cache_list_size++;
    i->refcount = 1;

    mutex_unlock(&vfs_cache_lock);
    return i;
}

void inode_free(struct inode* i)
{
    if (!i) {
        return;
    }
    mutex_lock(&vfs_cache_lock);
    
    if (i->refcount > 1) {
        i->refcount--;
        mutex_unlock(&vfs_cache_lock);
        return;
    }

    if (cache_list_size > CACHE_FLUSH_THRESHOLD) {
        //TODO: implement eviction policy
    }
    mutex_unlock(&vfs_cache_lock);
}

int vfs_get_fsid()
{
    static int fsid = 0;
    return fsid++;
}

const struct file_ops* fs_table[4] = {
    &fatfs_file_ops
};

const char* fs_name_table[4] = {
    "fatfs"
};

struct inode vfs_root;

struct file vfs_file_table[VFS_MAXFILES];
uint8_t file_table_free[VFS_MAXFILES];
uint8_t file_table_free_top;

void vfs_init()
{
    free_list = NULL;
    cache_list = NULL;
    for (int i = 0; i < VFS_MAXFILES; i++) {
        file_table_free[file_table_free_top++] = i;
    }
}


struct file* file_alloc()
{
    if (file_table_free_top == 0) {
        return NULL;
    }
    return &vfs_file_table[file_table_free[--file_table_free_top]];
}

void file_free(struct file* f)
{
    if (file_table_free_top == VFS_MAXFILES)
    {   //very bad, means memory was leaked
        return;
    }
    file_table_free[file_table_free_top++] = f - vfs_file_table;
}

struct file_ops* get_filesystem(const char* name)
{
    for (int i = 0; i < 4; i++) {
        if (strncmp(fs_name_table[i], name, 4) == 0) {
            return (struct file_ops*) fs_table[i];
        }
    }
    return NULL;
}

//this assumus that a lock has been achieved prior to calling
struct inode* vfs_lookup(struct inode* dir, char* name)
{
    struct inode* current = cache_list;
    
    if (strncmp(name, ".", FS_INAME_LEN) == 0 || *name == '\0') {
        return dir;
    }
    if (strncmp(name, "..", FS_INAME_LEN) == 0) { //find the parent inode
        while (current) {
            if (current->file == dir->dir) {
                current->refcount++;
                return current;
            }
            current = current->next;
        }
        //not in cache so do lookup
        return dir->fs->fops->lookupn(dir, name);
    }

    while (current) {
        if (strncmp(current->name, name, FS_INAME_LEN) == 0 && current->dir == dir->dir) {
            current->refcount++;
            return current;
        }
        current = current->next;
    }
    
    return dir->fs->fops->lookupn(dir, name);
}


struct inode* vfs_walk_path(const char* path)
{
    struct inode* current_dir;
    int path_idx = 0;
    char path_cpy[FS_PATH_LEN+1];
    char* current_word = path_cpy;

    mutex_lock(&vfs_cache_lock);
    
    //absolute path
    if (*path == '/') {
        current_dir = &vfs_root;
        path_idx++;
    } else {
        current_dir = current_process->cwd;
    }
    strlcpy(path_cpy + path_idx, path + path_idx, FS_PATH_LEN);

    while (path[path_idx]) {
        if (path[path_idx] == '/') {
            path_cpy[path_idx] = '\0';
            
            struct inode* next = vfs_lookup(current_dir, current_word);
            if (!next) {
                inode_free(current_dir);
                return NULL;
            }

            inode_free(current_dir);
            current_dir = next;
            current_word = path_cpy + path_idx + 1;
        }
        path_idx++;
    }

    mutex_unlock(&vfs_cache_lock);
    return current_dir;
}

int check_access(cred_t credentials, struct permissions* perm, int mode)
{
    return 1;
}

ssize_t vfs_read(int fd, void* buffer, size_t count)
{
    mutex_lock(&vfs_cache_lock);
    struct file* f = proc_fd_get(current_process, fd);
    if (!f) return -EBADF;
    if (!(f->flags & O_RDONLY)) return -EBADF;

    ssize_t n = f->i->fs->fops->read(f, buffer, count);
    f->offset += n;
    mutex_unlock(&vfs_cache_lock);
    return n;
}

ssize_t vfs_write(int fd, const void* buffer, size_t count)
{
    mutex_lock(&vfs_cache_lock);
    struct file* f = proc_fd_get(current_process, fd);
    if (!f) return -EBADF;
    if (!(f->flags & O_WRONLY)) return -EBADF;

    ssize_t n = f->i->fs->fops->write(f, buffer, count);
    f->offset += n;
    mutex_unlock(&vfs_cache_lock);
    return n;

}

int vfs_open(const char* path, int flags)
{
    struct inode* i = vfs_walk_path(path);
    if (!i) return -ENOENT;

    struct file* f = file_alloc();
    if (!f) return -ENFILE;

    f->flags = flags;
    f->i = i;
    f->offset = 0;

    return proc_fd_add(current_process, f);
}

int vfs_close(int fd)
{

}

int vfs_fstat(int fd, struct stat *statbuf)
{

}

off_t vfs_lseek(int fd, off_t offset, int whence)
{

}

int vfs_ioctl(int fd, int cmd, void* arg)
{

}

int vfs_access(const char* path, int mode)
{

}

int vfs_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{

}

int vfs_fcntl(int fd, int op, int arg)
{

}

int vfs_flock(int fd, int op)
{

}

int vfs_fsync(int fd)
{

}

int vfs_ftruncate(int fd, off_t lenght)
{

}

int vfs_chdir(const char* path)
{

}

int vfs_mkdir(const char* path, mode_t mode)
{

}

int vfs_rmdir(const char* path)
{

}

int vfs_rename(const char* oldpath, const char* newpath)
{

}

int vfs_unlink(const char* path)
{

}

int vfs_symlink(const char* target, const char* linkpath)
{

}

int vfs_readlink(const char* path, char* buf, size_t bufsiz)
{

}

int vfs_fstatfs(int fd, struct statfs* buf)
{

}

int vfs_fchmod(int fd, mode_t mode)
{

}

int vfs_fchown(int fd, uid_t owner, gid_t group)
{

}

int vfs_sync()
{

}

int vfs_mount(const char* source, const char* target, const char* filesystemtype, int mountflags)
{

}

int vfs_umount(const char* target)
{

}



