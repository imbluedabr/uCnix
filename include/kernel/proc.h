#pragma once
#include <uapi/sys/types.h>
#include <kernel/waiter.h>
#include <fs/vfs.h>
#include <board/board.h>

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

    process_state_e state;
    pid_t pid;
    pid_t ppid;
    pid_t pgrp;
    cred_t credentials;
    uint32_t sigmask;

    struct inode* cwd;

    waiter_t* waiter;
    struct proc* wait_next; //next item in the wait queue
};

extern struct proc* current_process;
#define PROC_TABLE_LEN 4

void proc_init();
struct proc* proc_create(uint8_t* ustack, uint8_t* kstack, uint32_t size, void (*entry_point)());

void proc_unblock_process(pid_t process); //unblock a process
void proc_block(); //block the current process

//starts the scheduler and never returns
void proc_start_scheduling();

