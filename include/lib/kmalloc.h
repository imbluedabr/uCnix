#include <kernel/alloc.h>

extern uint8_t __heap_start[];
extern struct mem_alloc kernel_allocator;

void kmalloc_init(void* heap_base, int heap_size);
void* kmalloc(int size);
void* kzalloc(int size);
void kfree(void* ptr);



