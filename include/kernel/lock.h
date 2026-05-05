#pragma once
#include <stdint.h>
#include <kernel/waiter.h>
#include <uapi/sys/types.h>

//locking mechanisms used by allocators and filesystems
struct proc;

typedef struct {
    waiter_t waiter;
    pid_t owner;
    uint8_t count : 7;
    uint8_t lock : 1;
} mutex_t;

void mutex_init(mutex_t* mut);
void mutex_lock(mutex_t* mut);
void mutex_unlock(mutex_t* mut);



