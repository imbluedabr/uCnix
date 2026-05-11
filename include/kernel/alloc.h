#pragma once
#include <stdbool.h>
#include <stdint.h>

struct block {
    uint16_t size : 15; //max heap size is 32KiB
    uint16_t occupied : 1; //bit fields require the same base storage type(so i heared)
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

#define BLOCK_ARRAY_LEN 64
#define BLOCK_NIL 255

extern uint8_t __heap_start[];

//init heap for kmalloc
void heap_init(void* base, int size);

void heap_stat(struct memstat* buff);

//allocate a contiguous piece of memory in sram, returns NULL on failure
void* kmalloc(int size);

//same as kmalloc except memory is initialized to 0
void* kzalloc(int size);

//free memory allocated with kmalloc
void kfree(void* ptr);


