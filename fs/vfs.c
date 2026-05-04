#include <fs/vfs.h>
#include <kernel/alloc.h>
#include <kernel/interrupt.h>

struct inode* free_list;
struct inode* cache_list;
uint8_t cache_list_size;
uint8_t free_list_size;

//now i have a very simple eviction policy that just cleans the cache when it reaches the threshold
#define CACHE_FLUSH_THRESHOLD 16

struct inode* inode_alloc()
{

    if (!free_list) {
        return kmalloc(sizeof(struct inode));
    }

    int irqstat = disable_interrupts();

    struct inode* i = free_list;
    free_list = i->next;
    i->next = cache_list;
    cache_list = i;
    free_list_size--;
    cache_list_size++;

    enable_interrupts(irqstat);
    return i;
}

void inode_free(struct inode* i)
{
    if (!i) {
        return;
    }
    int irqstat = disable_interrupts();
    
    if (i->refcount > 1) {
        i->refcount--;
        enable_interrupts(irqstat);
        return;
    }

    if (cache_list_size > CACHE_FLUSH_THRESHOLD) {
        //TODO: implement eviction policy
    }
    enable_interrupts(irqstat);
}

void vfs_init()
{
    free_list = NULL;
    cache_list = NULL;
}


struct inode root;


struct inode* vfs_lookup(const char* path)
{
    return NULL;
}



