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
    uint16_t blk_shift;
    mutex_t alloc_mut;
};

struct block {
    uint16_t size : 15; //max heap size is (2^15)*(2^blk_shift)
    uint16_t occupied : 1; //bit fields require the same base storage type
    uint8_t prev;
    uint8_t next;
};

struct memstat {
    uint8_t blocks_used;
    uint8_t blocks_total;
    uint8_t fragmentation;
    uint32_t bytes_used;
    uint32_t bytes_total;
};

#define BLOCK_NIL 255

//init heap for kmalloc
void heap_init(struct mem_alloc* heap, struct block* blk_array, uint8_t* unused_blocks, void* base, int size, int blk_count, int blk_shift);
void heap_stat(struct mem_alloc* heap, struct memstat* buff);

void* heap_alloc(struct mem_alloc* heap, int size);
void heap_free(struct mem_alloc* heap, void* ptr);

