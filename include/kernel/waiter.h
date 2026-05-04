#pragma once

typedef struct {
    struct proc* head;
    struct proc* tail;
} waiter_t;


//enqueue a process to a waiter
void waiter_push(waiter_t* waiter, struct proc* process);

//dequeue a process from a waiter
struct proc* waiter_pop(waiter_t* waiter);

//remove a process from a waiter
void waiter_remove(waiter_t* waiter, struct proc* process);

