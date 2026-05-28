#include <lib/kmalloc.h>
#include <lib/stdlib.h>

struct mem_alloc kernel_allocator;
struct block blk_array[64];
uint8_t unused_blocks[64];

void kmalloc_init(void* heap_base, int heap_size)
{
    heap_init(&kernel_allocator, blk_array, unused_blocks, heap_base, heap_size, 64);
}

inline void* kmalloc(int size)
{
    return heap_alloc(&kernel_allocator, size);
}

inline void* kzalloc(int size)
{
    uint8_t* ptr = heap_alloc(&kernel_allocator, size);
    if (!ptr) return ptr;
    memset(ptr, 0, size);
    return ptr;
}

inline void kfree(void* ptr)
{
    heap_free(&kernel_allocator, ptr);
}

