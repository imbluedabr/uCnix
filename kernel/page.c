#include <kernel/page.h>
#include <kernel/alloc.h>

struct mem_alloc userspace_allocator;
static struct block blk_array[8];
static uint8_t unused_blocks[8];

void page_init()
{
    heap_init(&userspace_allocator, blk_array, unused_blocks, __userspace_start, __userspace_end - __userspace_start, 8, 4);
}

void* page_alloc(int size)
{    
    return heap_alloc(&userspace_allocator, size);
}

void page_free(void* ptr)
{
    heap_free(&userspace_allocator, ptr);
}




