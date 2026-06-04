#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>
#include <kernel/device.h>
#include <kernel/panic.h>
#include <lib/stdlib.h>
#include <lib/kmalloc.h>
#include <lib/kprint.h>
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

[[gnu::aligned(8)]] stack_t stack_array[PROC_MAX_PROC];
uint8_t stack_free_list[PROC_MAX_PROC];
uint8_t stack_free_list_top;

inline stack_t* proc_stack_alloc()
{
    if (stack_free_list_top == 0) return NULL;
    return &stack_array[stack_free_list[--stack_free_list_top]];
}

inline void proc_stack_free(stack_t* stack)
{
    if (stack_free_list_top > 4) thread_panic("stack memory leak!");
    stack_free_list[stack_free_list_top++] = stack - stack_array; 
}

inline pid_t proc_pid_alloc()
{
    if (proc_pid_free_list_top == 0) return 255;
    return proc_pid_free_list[--proc_pid_free_list_top];
}

inline void proc_pid_free(pid_t pid)
{
    if (proc_pid_free_list_top > PROC_MAX_PROC) thread_panic("bruh...");
    proc_pid_free_list[proc_pid_free_list_top++] = pid;
}

inline int proc_enqueue(struct proc* process)
{
    if (proc_run_queue_count == PROC_MAX_PROC) return -1;
    
    proc_run_queue[proc_run_queue_head] = process;
    proc_run_queue_head = (proc_run_queue_head + 1) & (PROC_MAX_PROC - 1);
    proc_run_queue_count++;
    
    return 0;
}

inline struct proc* proc_dequeue()
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

inline int proc_fd_add(struct proc* p, struct file* f)
{
    int fd = proc_fd_alloc(p);
    if (fd < 0) return fd;
    p->local_fd_table[fd] = f - vfs_file_table;
    return fd;
}

inline int proc_fd_set(struct proc* p, int fd, struct file* f)
{
    int idx = f - vfs_file_table;
    p->local_fd_table[fd] = idx;
    return idx;
}


inline struct proc* proc_get_process(pid_t pid)
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



inline void proc_free_process(struct proc* p)
{
    mutex_lock(&proc_acces_lock);
    
    if (--p->refcount > 0) {
        mutex_unlock(&proc_acces_lock);
        return;
    }

    struct proc* current = proc_active_list;
    if (current == p) {
        proc_active_list = p->next;
        p->next = proc_free_list;
        proc_free_list = p;
        mutex_unlock(&proc_acces_lock);
        return;
    }

    while(current->next) {
        if (current->next == p) {
            current->next = p->next;
            p->next = proc_free_list;
            proc_free_list = p;
            break;
        }
        current = current->next;
    }

    mutex_unlock(&proc_acces_lock);
}

inline struct proc* proc_alloc_process()
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
    
    proc_sched_init();
}

int proc_reap(struct proc* p)
{
    if (p->state != PROC_ZOMBIE) return -255;
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

int proc_kill(pid_t pid, int sig)
{
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    proc_stop_scheduling();
    kdbg("sending sig %d\n", sig);
    int action = 0;
    if (p->sigmask & (1 << sig)) {
        p->exit_code = sig;
        switch(sig) {
            case SIGCHLD: //sigchld wakes up the parent of a child process
            case SIGCONT:
                if (p->state == PROC_BLOCKED) {
                    action = 1;
                }
                break;
            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                if (p->state == PROC_READY) {
                    action = 2;
                }
                break;
            default:
                if (p->state != PROC_ZOMBIE) {
                    action = 3;
                }
        }

    }
    
    proc_free_process(p);
    if (action == 1) {
        proc_unblock_process(p);
    } else if (action == 2) {
        proc_block(p);
    } else if (action == 3) {
        proc_mark_zombie(p, 128 + sig);
        proc_restart_scheduling();
    } else {
        proc_restart_scheduling();
    }
    return 0;
}


