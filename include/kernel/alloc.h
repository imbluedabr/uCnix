#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <kernel/lock.h>

struct mem_alloc {
    struct block* blk;
    void* block_base;
    uint8_t* unused_blocks;
    uint8_t unused_blocks_top;
    uint8_t blk_count;
    mutex_t alloc_mut;
};

struct block {
    uint16_t size : 15; //max heap size is 32KiB
    uint16_t occupied : 1; //bit fields require the same base storage type
    uint8_t prev;
    uint8_t next;
};

struct memstat {
    uint16_t blocks_used;
    uint16_t blocks_total;
    uint16_t bytes_used;
    uint16_t bytes_total;
    uint16_t fragmentation;
};

#define BLOCK_NIL 255

//init heap for kmalloc
void heap_init(struct mem_alloc* heap, struct block* blk_array, uint8_t* unused_blocks, void* base, int size, int blk_count);
void heap_stat(struct mem_alloc* heap, struct memstat* buff);

void* heap_alloc(struct mem_alloc* heap, int size);
void heap_free(struct mem_alloc* heap, void* ptr);

