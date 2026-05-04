#pragma once
#include <stdint.h>
#include <kernel/waiter.h>

//locking mechanisms used by allocators and filesystems
struct proc;

typedef struct {
    waiter_t waiter;
    uint8_t lock;
} mutex_t;

void mutex_init(mutex_t* mut);
void mutex_lock(mutex_t* mut);
void mutex_unlock(mutex_t* mut);



