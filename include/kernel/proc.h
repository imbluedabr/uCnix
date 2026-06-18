#pragma once
#include <uapi/sys/types.h>
#include <kernel/waiter.h>
#include <kernel/lock.h>
#include <kernel/settings.h>
#include <fs/vfs.h>
#include <board/board.h>
#include <stdbool.h>

typedef enum : uint8_t {
    PROC_ZOMBIE,
    PROC_BLOCKED,
    PROC_READY
} process_state_e;

typedef struct {
    gid_t rgid;
    gid_t egid;
    uid_t ruid;
    uid_t euid;
    gid_t groups[4];
} cred_t;

struct proc {
#ifdef __ARM_ARCH_8M_MAIN__
    uint8_t* psp;
    CONTROL_Type control;
    uint8_t* splim;
    //these either save the user context or kernel context when switching to and from syscall handler code when the user does an SVC
    uint8_t* save_psp;
    CONTROL_Type save_control;
    uint8_t* save_splim;
#endif
    uint8_t kernel_mode; //0 = user mode, 1 = kernel mode
    uint8_t crit_section;
    uint8_t refcount;

    uint8_t* program_base;
    int program_size;

    process_state_e state;
    uint8_t pid;
    uint8_t ppid;
    uint8_t pgrp;
    uint32_t sigmask;
    int exit_code;

    cred_t credentials;
    uint8_t local_fd_table[PROC_MAXFILES];

    //current working directory of the process
    struct inode* cwd;
    
    waiter_t* waiting_on;
    struct proc* wait_next; //next item in the wait queue
    struct proc* next; //next process in the process linked list

};

typedef struct {
    uint8_t* user_stack;
    size_t size;
    void (*entry_point)();
    const char** argv;
    uint8_t stopped : 1;
    uint8_t kernel_mode : 1;
} process_desc_t;

typedef uint8_t stack_t[PROC_KSTACK_SIZE];

extern mutex_t proc_acces_lock;
extern struct proc* current_process;
extern struct proc* proc_active_list;
extern bool proc_sched_started;

void proc_init();

stack_t* proc_stack_alloc();
void proc_stack_free(stack_t* stack);

pid_t proc_pid_alloc();
void proc_pid_free(pid_t pid);

int proc_enqueue(struct proc* process);
struct proc* proc_dequeue();

int proc_fd_alloc(struct proc* p);
void proc_fd_free(struct proc* p, int fd);
int proc_fd_add(struct proc* p, struct file* f);
struct file* proc_fd_get(struct proc* p, int fd);

struct proc* proc_get_process(pid_t pid);
void proc_free_process(struct proc* p);
struct proc* proc_alloc_process();
int proc_fd_add(struct proc* p, struct file* f);
int proc_fd_set(struct proc* p, int fd, struct file* f);

int proc_kill(pid_t pid, int sig); //send a signal to a process
int proc_reap(struct proc* p);

void proc_sched_init();
struct proc* proc_create(process_desc_t* descriptor);
void proc_mark_zombie(struct proc* p, int exit_code);
void proc_unblock_process(struct proc* p); //unblock a process
void proc_block(struct proc* p); //block a process
void proc_schedule(); //trigger the scheduler
//starts the scheduler and never returns
void proc_start_scheduling();
//stops the scheduler
struct proc* proc_stop_scheduling();
void proc_restart_scheduling();

