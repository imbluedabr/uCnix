#include <kernel/waiter.h>
#include <kernel/proc.h>
#include <stddef.h>

//enqueue a process to a waiter
void waiter_push(waiter_t* waiter, struct proc* process)
{
    if (!waiter->head) { //if head is null there are no items in the list
        waiter->head = process;
        waiter->tail = process;
    } else {
        waiter->head->wait_next = process;
        waiter->head = process;
    }
    process->waiter = waiter; //store which waiter the process is blocked on
}

//dequeue a process from a waiter
struct proc* waiter_pop(waiter_t* waiter)
{
    if (!waiter->tail) {
        return NULL;
    }

    struct proc* next = waiter->tail;
    waiter->tail = next->wait_next;
    next->wait_next = NULL;
    next->waiter = NULL;

    if (!waiter->tail) { //if there are no more items we also set the head to NULL
        waiter->head = NULL;
    }
    return next;
}

//remove a process from a waiter
void waiter_remove(waiter_t* waiter, struct proc* process)
{
    struct proc* curr = waiter->head;
    struct proc* prev = NULL;

    while (curr) {
        if (curr == process) {
            if (prev) { //remove the item from the list
                prev->wait_next = curr->wait_next;
            } else {
                waiter->head = curr->wait_next;
            }
            
            if (!curr->wait_next) {
                waiter->tail = prev;
            }

            break;
        }
        prev = curr;
        curr = curr->wait_next;
    }

}

