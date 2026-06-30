#pragma once
#include <stdint.h>

extern uint8_t __userspace_start[];
extern uint8_t __userspace_end[];
extern struct mem_alloc userspace_allocator;

void page_init();
void* page_alloc(int size);
void page_free(void* ptr);



