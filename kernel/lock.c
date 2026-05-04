#include <kernel/lock.h>
#include <kernel/interrupt.h>
#include <kernel/proc.h>
#include <stddef.h>

void mutex_init(mutex_t* mut)
{
    mut->waiter.head = NULL;
    mut->waiter.tail = NULL;
    mut->lock = 0;
}

void mutex_lock(mutex_t* mut)
{
    int irq = disable_interrupts();
    
    //if the mutex is locked we add the process to the wait queue and then block it
    if (mut->lock) {
        waiter_push(&mut->waiter, current_process);
        enable_interrupts(irq);
        proc_block();
        
        //if we get unblocked that means that we get the lock 
        return;
    }
    
    //claim the lock
    mut->lock = 1;

    enable_interrupts(irq);
}

void mutex_unlock(mutex_t* mut)
{
    
    int irq = disable_interrupts();
    
    //pop the next process of the wait queue
    struct proc* next = waiter_pop(&mut->waiter);
    if (next) {
        enable_interrupts(irq);
        proc_unblock_process(next->pid);
        return;
    }

    //if there are no items on the wait queue unclaim the lock
    mut->lock = 0;

    enable_interrupts(irq);
}

void mutex_remove(mutex_t* mut, struct proc* process) {
    int irq = disable_interrupts();
    waiter_remove(&mut->waiter, process);
    enable_interrupts(irq);
}

