#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>
#include <kernel/device.h>
#include <kernel/panic.h>
#include <lib/stdlib.h>
#include <lib/kmalloc.h>
#include <arch/armv8-m/proc.h>
#include <arch/armv8-m/syscall.h>
#include <fs/vfs.h>
#include <uapi/sys/errno.h>
#include <uapi/signal.h>
#include <stddef.h>

void proc_sched_init()
{
    __disable_irq();
    
    register_interrupt(PendSV_IRQn, NULL, pendsv_handler);

    NVIC_SetPriority(PendSV_IRQn, 7);

    __enable_irq();
}

struct proc* proc_create(process_desc_t* descriptor)
{
    struct proc* p = proc_alloc_process();
    
    mutex_lock(&p->lock);

    pid_t pid = proc_pid_alloc();
    p->pid = pid;

    //user mode thread
    if (!descriptor->kernel_mode) {
        p->psp = descriptor->user_stack + descriptor->size; //the stack grows downwards so we must put the top of the array into psp
        p->splim = descriptor->user_stack; //the limit of the stack is the base of the stack memory
        
        p->control.w = CONTROL_SPSEL_Msk | CONTROL_nPRIV_Msk;
    } else {
        uint8_t* kstack = (uint8_t*) proc_stack_alloc();
        p->psp = kstack + PROC_KSTACK_SIZE; //the stack grows downwards so we must put the top of the array into psp
        p->splim = kstack; //the limit of the stack is the base of the stack memory
        
        p->control.w = CONTROL_SPSEL_Msk;
    }
    
    //kernel mode thread
    if (!descriptor->kernel_mode) {
        uint8_t* kstack = (uint8_t*) proc_stack_alloc();
        p->save_psp = kstack + PROC_KSTACK_SIZE;
        p->save_splim = kstack;
        p->save_control.w = CONTROL_SPSEL_Msk;
    }

    //setup user mode stack
    p->psp -= sizeof(struct context_frame);
    struct context_frame* frame = (struct context_frame*) p->psp;
    memset(frame, 0, sizeof(struct context_frame));
    frame->exc_return = 0xFFFFFFBC;
    frame->base_frame.pc = (uint32_t) descriptor->entry_point;
    frame->base_frame.cpsr.b.T = 1;

    //setup kernel mode stack
    if (!descriptor->kernel_mode) {
        p->save_psp -= sizeof(struct context_frame);
        frame = (struct context_frame*) p->save_psp;
        memset(frame, 0, sizeof(struct context_frame));
        frame->exc_return = 0xFFFFFFBC;
        frame->base_frame.pc = (uint32_t) &svc_thread;
        frame->base_frame.cpsr.b.T = 1;
    }

    memset(p->local_fd_table, 255, PROC_MAXFILES);
    
    if (descriptor->stopped) {
        p->state = PROC_BLOCKED;
    } else {
        p->state = PROC_READY;
        proc_enqueue(p);
    }

    mutex_unlock(&p->lock);
    return p;
}

void proc_mark_zombie(struct proc* p, int exit_code)
{
    if (p->waiting_on) {
        waiter_remove(p->waiting_on, p);
    }
    __disable_irq();
    p->exit_code = exit_code;
    p->state = PROC_ZOMBIE;
    __enable_irq();
}

void proc_unblock_process(struct proc* p)
{
    __disable_irq();
    p->state = PROC_READY;
    proc_sched_started = true;
    proc_enqueue(p);
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    proc_restart_scheduling();
    __WFI();
}

void proc_block(struct proc* p)
{
    __disable_irq();
    p->state = PROC_BLOCKED;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    proc_restart_scheduling();
    __WFI();
    //wait until pendsv fires
}

void proc_schedule()
{
    __disable_irq();
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    proc_restart_scheduling();
    __WFI();
}

void proc_sched_new_task()
{
    if (proc_sched_started == 0) return;

    if (current_process->state == PROC_READY) {
        proc_enqueue(current_process);
    }
    struct proc* p;
    do {
        p = proc_dequeue();
        if (!p) {
            __BKPT();
        }
    } while(p->state != PROC_READY);
    current_process = p;
}

[[noreturn]] void proc_start_scheduling()
{
    __disable_irq();
    current_process = proc_dequeue();
    SysTick->VAL = 0; //reset systick timer to prevent timer interrupt when the svc hasnt executed yet
    __asm volatile
    (
        "   cpsie i\t\n"
        "   svc #0\t\n"
    );
    while(1);
}

struct proc* proc_stop_scheduling()
{
    __disable_irq();
    proc_sched_started = 0;
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
    __DSB();
    __enable_irq();
    return current_process;
}

void proc_restart_scheduling()
{
    __disable_irq();
    proc_sched_started = 1;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
    __DSB();
    __enable_irq();
}

