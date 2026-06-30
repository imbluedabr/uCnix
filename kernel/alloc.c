#include <kernel/alloc.h>
#include <kernel/lock.h>
#include <lib/stdlib.h>
#include <lib/kprint.h>
#include <stddef.h>

static inline struct block* get_block(struct mem_alloc* heap)
{
    if (heap->unused_blocks_top == 0) {
        return NULL;
    }
    uint8_t idx = heap->unused_blocks[--heap->unused_blocks_top];
    return &heap->blk[idx];
}

static inline int get_block_idx(struct mem_alloc* heap, struct block* b)
{
    return b - heap->blk;
}

static inline struct block* get_block_addr(struct mem_alloc* heap, int idx)
{
    return &heap->blk[idx];
}

static inline void put_block(struct mem_alloc* heap, struct block* b)
{
    uint8_t idx = b - heap->blk;
    heap->unused_blocks[heap->unused_blocks_top++] = idx;
}

void heap_init(struct mem_alloc* heap, struct block* blk_array, uint8_t* unused_blocks, void* base, int size, int blk_count, int blk_shift)
{
    mutex_init(&heap->alloc_mut);
    heap->block_base = base;
    heap->blk = blk_array;
    heap->blk[0].size = size >> blk_shift; //so the block shift basicly sets a unit size, so instead of the size of a block being expressed in bytes its expressed as units of 2^blk_shift. this allows the allocator to address more memory then the limit of the size member
    heap->blk[0].occupied = false;
    heap->blk[0].next = BLOCK_NIL;
    heap->blk[0].prev = BLOCK_NIL;
    heap->unused_blocks_top = 0;
    heap->blk_count = blk_count;
    heap->blk_shift = blk_shift;
    heap->unused_blocks = unused_blocks;
    for (int i = 1; i < blk_count; i++) {
        heap->unused_blocks[heap->unused_blocks_top++] = i;
    }
}

void heap_stat(struct mem_alloc* heap, struct memstat* buff)
{
    struct block* current = get_block_addr(heap, 0);
    bool prev_occupied = true;
    memset(buff, 0, sizeof(struct memstat));
    
    while (current) {

        if (prev_occupied != current->occupied) {
            buff->fragmentation++;
        }
        prev_occupied = current->occupied;
        
        if (current->occupied) {
            buff->blocks_used++;
            buff->bytes_used += current->size << heap->blk_shift;
        }
        
        buff->blocks_total++;
        buff->bytes_total += current->size << heap->blk_shift;

        if (current->next == BLOCK_NIL) {
            return;
        }
        current = get_block_addr(heap, current->next);
    }
}

void* heap_alloc(struct mem_alloc* heap, int size)
{
    mutex_lock(&heap->alloc_mut);
    size = (size + ((1 << heap->blk_shift) - 1)) >> heap->blk_shift; //round to block unit size
    struct block* current = get_block_addr(heap, 0);
    char* base = heap->block_base;
    do {
        if (current->size > size && !current->occupied) {
            
            struct block* new = get_block(heap);
            if (new == NULL) {
                base = NULL;
                break;
            }
            
            new->prev = get_block_idx(heap, current);
            new->next = current->next;
            if (current->next != BLOCK_NIL) {
                struct block* new_next = get_block_addr(heap, current->next);
                new_next->prev = get_block_idx(heap, new);
            }
            new->occupied = false;
            new->size = current->size - size;
            current->size = size;
            current->occupied = true;
            current->next = get_block_idx(heap, new);
            break;
        }
        if (current->size == size && !current->occupied) {
            current->occupied = true;
            break;
        }
        if (current->next == BLOCK_NIL) {
            base = NULL;
            break;
        }
        base += current->size << heap->blk_shift;
        current = get_block_addr(heap, current->next);
    } while(1);

    mutex_unlock(&heap->alloc_mut);
    return base;
}

static inline void try_coalesce(struct mem_alloc* heap, struct block* b)
{
    if (b->next != BLOCK_NIL) {
        struct block* next = get_block_addr(heap, b->next);

        if (next->occupied == false) {
            b->size += next->size;
            b->next = next->next;
            if (b->next != BLOCK_NIL) {
                struct block* new_next = get_block_addr(heap, b->next);
                new_next->prev = get_block_idx(heap, b);
            }
            put_block(heap, next);
        }
    }

    if (b->prev != BLOCK_NIL) {
        struct block* prev = get_block_addr(heap, b->prev);

        if (prev->occupied == false) {
            prev->next = b->next;
            prev->size += b->size;
            if (b->next != BLOCK_NIL) {
                struct block* new_next = get_block_addr(heap, b->next);
                new_next->prev = b->prev;
            }
            put_block(heap, b);
        }
    }

}

void heap_free(struct mem_alloc* heap, void* ptr)
{
    mutex_lock(&heap->alloc_mut);
    struct block* current = get_block_addr(heap, 0);
    char* base = heap->block_base;
    do {
        
        if (base == ptr) {
            current->occupied = false;
            try_coalesce(heap, current);
            break;
        }

        if (current->next == BLOCK_NIL) {
            break;
        }
        base += current->size << heap->blk_shift;
        current = get_block_addr(heap, current->next);
    } while(1);
    mutex_unlock(&heap->alloc_mut);
}

