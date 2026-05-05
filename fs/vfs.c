#include <fs/vfs.h>
#include <kernel/alloc.h>
#include <kernel/interrupt.h>
#include <kernel/lock.h>
#include <kernel/proc.h>
#include <lib/stdlib.h>

static struct inode* free_list;
static struct inode* cache_list;
static uint8_t cache_list_size;
static uint8_t free_list_size;
static mutex_t cache_lock;

#define CACHE_FLUSH_THRESHOLD 16

/* The inode cache works by first keeping a free list of free inodes. Then when an inode is allocated its removed from the free list and added to the cache list.
 * When an inodes refcount reaches 0 we keep it inside the cache list so its cached, then when the cache gets too big we flush it and only then will the inodes return to the original free list.
*/

struct inode* inode_alloc()
{
    mutex_lock(&cache_lock);
    if (!free_list) {
        return kmalloc(sizeof(struct inode));
    }

    struct inode* i = free_list;
    free_list = i->next;
    i->next = cache_list;
    cache_list = i;
    free_list_size--;
    cache_list_size++;

    mutex_unlock(&cache_lock);
    return i;
}

void inode_free(struct inode* i)
{
    if (!i) {
        return;
    }
    mutex_lock(&cache_lock);
    
    if (i->refcount > 1) {
        i->refcount--;
        mutex_unlock(&cache_lock);
        return;
    }

    if (cache_list_size > CACHE_FLUSH_THRESHOLD) {
        //TODO: implement eviction policy
    }
    mutex_unlock(&cache_lock);
}

void vfs_init()
{
    free_list = NULL;
    cache_list = NULL;
}


struct file_ops* const fs_table[4] = {

};

const char* fs_name_table[4] = {
    "fatfs"
};

struct inode vfs_root;

struct file_ops* get_filesystem(const char* name)
{
    for (int i = 0; i < 4; i++) {
        if (strncmp(fs_name_table[i], name, 4) == 0) {
            return fs_table[i];
        }
    }
    return NULL;
}

struct inode* vfs_lookup(struct inode* dir, char* name)
{
    struct inode* current = cache_list;
    
    if (strncmp(name, ".", FS_INAME_LEN) == 0 || *name == '\0') {
        return dir;
    }
    if (strncmp(name, "..", FS_INAME_LEN) == 0) { //find the parent inode
        while (current) {
            if (current->file == dir->dir) {
                return current;
            }
            current = current->next;
        }
        //not in cache so do lookup
        return dir->fs->fops->lookupi(dir->fs, dir->dir);
    }

    while (current) {
        if (strncmp(current->name, name, FS_INAME_LEN) == 0 && current->dir == dir->dir) {
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

    mutex_lock(&cache_lock);
    
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
            
            current_dir = vfs_lookup(current_dir, current_word);
            if (!current_dir) {
                return NULL;
            }
            current_word = path_cpy + path_idx + 1;
        }
        path_idx++;
    }

    mutex_unlock(&cache_lock);
    return NULL;
}

void mount_root(struct device* dev, const char* fs_name)
{
    struct file_ops* fs = get_filesystem(fs_name);
    fs->mount(&vfs_root, dev, "");
}

void mount_devfs()
{
    struct inode* mountpoint = vfs_lookup(&vfs_root, "dev");
    struct file_ops* fs = get_filesystem("devfs");
    fs->mount(mountpoint, NULL, "");
}


