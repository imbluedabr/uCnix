#include <kernel/page.h>
#include <kernel/alloc.h>

struct mem_alloc userspace_allocator;
static struct block blk_array[8];
static uint8_t unused_blocks[8];

void page_init()
{
    heap_init(&userspace_allocator, blk_array, unused_blocks, __userspace_start, 0x6000, 8);
}

void* page_alloc(int size)
{
    if (size & 0b11) size = (size & ~0b11) + 4;
    
    return heap_alloc(&userspace_allocator, size);
}

void page_free(void* ptr)
{
    heap_free(&userspace_allocator, ptr);
}




