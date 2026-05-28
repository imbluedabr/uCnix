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

mutex_t proc_acces_lock;
struct proc* proc_free_list;
struct proc* proc_active_list;

uint8_t proc_pid_free_list[PROC_MAX_PROC];
uint8_t proc_pid_free_list_top;

struct proc* proc_run_queue[PROC_MAX_PROC];
uint8_t proc_run_queue_head;
uint8_t proc_run_queue_tail;
uint8_t proc_run_queue_count;

struct proc* current_process;
bool proc_sched_started;

typedef uint8_t stack_t[PROC_KSTACK_SIZE];
[[gnu::aligned(8)]] stack_t stack_array[PROC_MAX_PROC];
uint8_t stack_free_list[PROC_MAX_PROC];
uint8_t stack_free_list_top;

static inline stack_t* stack_alloc()
{
    if (stack_free_list_top == 0) return NULL;
    return &stack_array[stack_free_list[--stack_free_list_top]];
}

static inline void stack_free(stack_t* stack)
{
    if (stack_free_list_top > 4) thread_panic("stack memory leak!");
    stack_free_list[stack_free_list_top++] = stack - stack_array; 
}

static inline pid_t proc_pid_alloc()
{
    if (proc_pid_free_list_top == 0) return 255;
    return proc_pid_free_list[--proc_pid_free_list_top];
}

static inline void proc_pid_free(pid_t pid)
{
    if (proc_pid_free_list_top > PROC_MAX_PROC) thread_panic("bruh...");
    proc_pid_free_list[proc_pid_free_list_top++] = pid;
}

static inline int proc_enqueue(struct proc* process)
{
    if (proc_run_queue_count == PROC_MAX_PROC) return -1;
    
    proc_run_queue[proc_run_queue_head] = process;
    proc_run_queue_head = (proc_run_queue_head + 1) & (PROC_MAX_PROC - 1);
    proc_run_queue_count++;
    
    return 0;
}

static inline struct proc* proc_dequeue()
{
    if (proc_run_queue_count == 0) return NULL;
    
    struct proc* process = proc_run_queue[proc_run_queue_tail];
    proc_run_queue_tail = (proc_run_queue_tail + 1) & (PROC_MAX_PROC - 1);
    proc_run_queue_count--;
    
    return process;
}

inline int proc_fd_alloc(struct proc* p)
{
    for (int i = 0; i < PROC_MAXFILES; i++) {
        if (p->local_fd_table[i] == 255) {
            return i;
        }
    }
    return -EMFILE;
}

inline void proc_fd_free(struct proc* p, int fd)
{
    p->local_fd_table[fd] = 255;
}

inline struct file* proc_fd_get(struct proc* p, int fd)
{
    if (fd == 255) return NULL;
    if (p->local_fd_table[fd] == 255) return NULL;
    return &vfs_file_table[p->local_fd_table[fd]];
}

static inline struct proc* proc_get_process(pid_t pid)
{
    mutex_lock(&proc_acces_lock);
    struct proc* current = proc_active_list;
    while(current) {
        if (current->pid == pid) {
            current->refcount++;
            mutex_unlock(&proc_acces_lock);
            return current;
        }
        current = current->next;
    }
    mutex_unlock(&proc_acces_lock);
    return NULL;
}

static inline void proc_free_process(struct proc* p)
{
    mutex_lock(&proc_acces_lock);
    
    if (--p->refcount > 0) {
        mutex_unlock(&proc_acces_lock);
        return;
    }

    struct proc* current = proc_active_list;
    while(current) {
        if (current->next == p) {
            current->next = p->next;
            p->next = proc_free_list;
            proc_free_list = p;
            break;
        }
    }

    mutex_unlock(&proc_acces_lock);
}

static inline struct proc* proc_alloc_process()
{
    mutex_lock(&proc_acces_lock);
    struct proc* p;
    if (proc_free_list) {
        p = proc_free_list;
        proc_free_list = p->next;
    } else {
        p = kzalloc(sizeof(struct proc));
        if (!p) thread_panic("could not allocate process!");
    }

    p->next = proc_active_list;
    proc_active_list = p;
    p->refcount = 1;

    mutex_unlock(&proc_acces_lock);
    return p;
}

inline int proc_fd_add(struct proc* p, struct file* f)
{
    int fd = proc_fd_alloc(p);
    if (fd < 0) return fd;
    p->local_fd_table[fd] = f - vfs_file_table;
    return fd;
}

void proc_init()
{
    proc_free_list = NULL;
    proc_active_list = NULL;
    
    proc_pid_free_list_top = 0;
    for (int i = 0; i < PROC_MAX_PROC; i++) {
        proc_pid_free_list[proc_pid_free_list_top++] = PROC_MAX_PROC - i - 1;
    }

    stack_free_list_top = 0;
    for (int i = 0; i < PROC_MAX_PROC; i++) {
        stack_free_list[stack_free_list_top++] = i;
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

struct proc* proc_create(process_desc_t* descriptor)
{
    struct proc* p = proc_alloc_process();
    
    mutex_lock(&p->lock);

    pid_t pid = proc_pid_alloc();
    p->state = PROC_READY;
    p->pid = pid;

    //user mode thread
    if (!descriptor->kernel_mode) {
        p->psp = descriptor->user_stack + descriptor->size; //the stack grows downwards so we must put the top of the array into psp
        p->splim = descriptor->user_stack; //the limit of the stack is the base of the stack memory
        
        p->control.w = CONTROL_SPSEL_Msk | CONTROL_nPRIV_Msk;
    } else {
        uint8_t* kstack = (uint8_t*) stack_alloc();
        p->psp = kstack + PROC_KSTACK_SIZE; //the stack grows downwards so we must put the top of the array into psp
        p->splim = kstack; //the limit of the stack is the base of the stack memory
        
        p->control.w = CONTROL_SPSEL_Msk;
    }
    
    //kernel mode thread
    if (!descriptor->kernel_mode) {
        uint8_t* kstack = (uint8_t*) stack_alloc();
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

    proc_enqueue(p);
    mutex_unlock(&p->lock);
    return p;
}

int proc_reap(struct proc* p)
{
    inode_free(p->cwd);
    for (int i = 0; i < PROC_MAXFILES; i++) {
        int index = p->local_fd_table[i];
        if (index == 255) continue;
        struct file* f = &vfs_file_table[index];
        file_free(f);
    }
    proc_pid_free(p->pid);
    proc_free_process(p);
    return p->exit_code;
}

void proc_mark_zombie(struct proc* p, int exit_code)
{
    if (p->waiting_on) {
        waiter_remove(p->waiting_on, p);
    }
    p->exit_code = exit_code;
    __disable_irq();
    p->state = PROC_ZOMBIE;
    __enable_irq();
}

int proc_kill(pid_t pid, int sig)
{
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    mutex_lock(&p->lock);
    
    if (p->sigmask & (1 << sig)) {

        switch(sig) {
            case SIGCHLD: //sigchld wakes up the parent of a child process
            case SIGCONT:
                if (p->state == PROC_BLOCKED) {
                    proc_unblock_process(p);
                }
                break;
            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                if (p->state == PROC_READY) {
                    proc_block(p);
                }
                break;
            default:
                if (p->state != PROC_ZOMBIE) {
                    proc_mark_zombie(p, 128 + sig);
                }
        }

    }

    mutex_unlock(&p->lock);
    proc_free_process(p);
    return 0;
}

void proc_unblock_process(struct proc* p)
{
    __disable_irq();
    p->state = PROC_READY;
    proc_enqueue(p);
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __enable_irq();
    __WFI();
}

void proc_block(struct proc* p)
{
    __disable_irq();
    p->state = PROC_BLOCKED;
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

    //disable the systick timer
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);

    struct proc* last_process = current_process;
    current_process = NULL;
    proc_sched_started = false;    
    return last_process;
}


