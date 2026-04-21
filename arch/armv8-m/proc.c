#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>
#include <kernel/device.h>
#include <lib/stdlib.h>
#include <arch/armv8-m/proc.h>
#include <arch/armv8-m/syscall.h>
#include <stddef.h>

struct proc proc_table[PROC_TABLE_LEN];
uint8_t proc_free_list[PROC_TABLE_LEN];
uint8_t proc_free_list_top;

struct proc* proc_run_queue[PROC_TABLE_LEN];
uint8_t proc_run_queue_head;
uint8_t proc_run_queue_tail;
uint8_t proc_run_queue_count;

struct proc* current_process;
bool proc_sched_started;

int proc_enqueue(struct proc* process)
{
    if (proc_run_queue_count == PROC_TABLE_LEN) return -1;
    
    proc_run_queue[proc_run_queue_head] = process;
    proc_run_queue_head = (proc_run_queue_head + 1) & (PROC_TABLE_LEN - 1);
    proc_run_queue_count++;
    
    return 0;
}

struct proc* proc_dequeue()
{
    if (proc_run_queue_count == 0) return NULL;
    
    struct proc* process = proc_run_queue[proc_run_queue_tail];
    proc_run_queue_tail = (proc_run_queue_tail + 1) & (PROC_TABLE_LEN - 1);
    proc_run_queue_count--;
    
    return process;
}



void proc_init()
{
    proc_free_list_top = 0;
    for (int i = 0; i < PROC_TABLE_LEN; i++) {
        proc_free_list[proc_free_list_top++] = i;
    }
    proc_run_queue_head = 0;
    proc_run_queue_tail = 0;
    proc_run_queue_count = 0;
    current_process = NULL;
    proc_sched_started = false;

    __disable_irq();
    
    register_interrupt(PendSV_IRQn, NULL, pendsv_handler);

    NVIC_SetPriority(PendSV_IRQn, 7);

    __enable_irq();
}

struct proc* proc_create(uint8_t* ustack, uint8_t* kstack, uint32_t stack_size, void (*entry_point)())
{
    pid_t pid = proc_free_list[--proc_free_list_top];
    struct proc* p = &proc_table[pid];
    p->state = PROC_READY;
    p->pid = pid;
    //user mode thread
    p->psp = ustack + stack_size; //the stack grows downwards so we must put the top of the array into psp
    p->splim = ustack; //the limit of the stack is the base of the stack memory
    p->control.w = CONTROL_SPSEL_Msk;
    
    //kernel mode thread
    p->save_psp = kstack + stack_size;
    p->save_splim = kstack;
    p->save_control.w = CONTROL_SPSEL_Msk;

    //setup user mode stack
    p->psp -= sizeof(struct context_frame);
    struct context_frame* frame = (struct context_frame*) p->psp;
    memset(frame, 0, sizeof(struct context_frame));
    frame->exc_return = 0xFFFFFFBC;
    frame->base_frame.pc = (uint32_t) entry_point;
    frame->base_frame.cpsr.b.T = 1;

    //setup kernel mode stack
    p->save_psp -= sizeof(struct context_frame);
    frame = (struct context_frame*) p->save_psp;
    memset(frame, 0, sizeof(struct context_frame));
    frame->exc_return = 0xFFFFFFBC;
    frame->base_frame.pc = (uint32_t) &svc_thread;
    frame->base_frame.cpsr.b.T = 1;

    proc_enqueue(p);
    return p;
}

void proc_unblock_process(pid_t process)
{
    __disable_irq();
    proc_table[process].state = PROC_READY;
    proc_enqueue(&proc_table[process]);
    __enable_irq();
}

void proc_block()
{
    __disable_irq();
    current_process->state = PROC_BLOCKED;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __enable_irq();
    __WFI();
    //wait until pendsv fires
}

void proc_sched_new_task()
{
    if (current_process->state == PROC_READY) {
        proc_enqueue(current_process);
    }
    current_process = proc_dequeue();
    if (current_process == NULL) { //we have to shutdown here
        __BKPT();
    }
}

void proc_start_scheduling()
{
    __disable_irq();
    current_process = proc_dequeue();
    SysTick->VAL = 0; //reset systick timer to prevent timer interrupt when the svc hasnt executed yet
    __asm volatile
    (
        "   cpsie i\t\n"
        "   svc #0\t\n"
    );
}

