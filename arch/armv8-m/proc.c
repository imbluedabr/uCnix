#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>
#include <lib/stdlib.h>
#include <arch/armv8-m/proc.h>
#include <stddef.h>

struct proc proc_table[PROC_TABLE_LEN];
uint8_t proc_free_list[PROC_TABLE_LEN];
uint8_t proc_free_list_top;

struct proc* proc_run_queue[PROC_TABLE_LEN];
uint8_t proc_run_queue_head;
uint8_t proc_run_queue_tail;
uint8_t proc_run_queue_count;

struct proc* current_process;
volatile uint32_t proc_ticks;


int proc_enqueue(struct proc* process)
{
    if (proc_run_queue_count == PROC_TABLE_LEN) return -1;
    
    proc_run_queue[proc_run_queue_tail] = process;
    proc_run_queue_tail = (proc_run_queue_tail + 1) & (PROC_TABLE_LEN - 1);
    proc_run_queue_count++;
    
    return 0;
}

struct proc* proc_dequeue()
{
    if (proc_run_queue_count == 0) return NULL;
    
    struct proc* process = proc_run_queue[proc_run_queue_head];
    proc_run_queue_head = (proc_run_queue_head + 1) & (PROC_TABLE_LEN - 1);
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

    __disable_irq();
    proc_ticks = 0;
    
    register_interrupt(SysTick_IRQn, NULL, proc_systick_handler);
    register_interrupt(PendSV_IRQn, NULL, pendsv_handler);
    __enable_irq();
}

struct proc* proc_create(uint8_t* stack, uint32_t stack_size, void (*entry_point)())
{
    struct proc* p = &proc_table[proc_free_list[--proc_free_list_top]];
    p->psp = stack + stack_size; //the stack grows downwards so we must put the top of the array into psp
    p->splim = stack; //the limit of the stack is the base of the stack memory
    p->entry_point = (uint32_t) entry_point;
    p->control = CONTROL_SPSEL_Msk;
    
    p->psp -= sizeof(struct context_frame);
    struct context_frame* frame = (struct context_frame*) p->psp;
    memset(frame, 0, sizeof(struct context_frame));
    frame->exc_return = 0xFFFFFFBC;
    frame->pc = (uint32_t) entry_point;
    frame->cpsr.w = xPSR_T_Msk;

    proc_enqueue(p);
    return p;
}

void proc_systick_handler()
{
    //__BKPT();
    proc_ticks++;
    //SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
}

uint32_t get_kernel_ticks()
{
    return proc_ticks;
}

void proc_sched_new_task()
{
    proc_enqueue(current_process);
    current_process = proc_dequeue();
}

void proc_start_scheduling()
{
    __disable_irq();
    SysTick_Config(48000);
    current_process = proc_dequeue();
    __asm volatile
    (
        "   cpsie i\t\n"
        "   svc #0\t\n"
    );
}

