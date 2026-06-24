#include <kernel/exec.h>
#include <kernel/tiny_exec.h>
#include <kernel/proc.h>
#include <kernel/page.h>
#include <kernel/interrupt.h>
#include <fs/vfs.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>

#include <uapi/sys/fcntl.h>
#include <uapi/sys/errno.h>
#include <uapi/sys/wait.h>
#include <uapi/signal.h>
#include <stdint.h>

#define BLOCK_MASK ((1 << SIGSTOP) | (1 << SIGKILL) | (1 << SIGSEGV) | (1 << SIGILL) | (1 << SIGFPE) | (1 << SIGBUS))

int check_owner(cred_t cred) {
    cred_t par_cred = current_process->credentials;
    if (
            par_cred.euid == 0 ||
            par_cred.euid == cred.euid ||
            par_cred.egid == cred.egid ||
            par_cred.euid == cred.ruid ||
            par_cred.egid == cred.rgid
            ) {
        return 1;
    }
    return 0;
}

__attribute__((optimize("O2"))) int sys_spawn(const char* path, fd_set* fd_list, const char** argv)
{
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) return fd;

    struct stat statbuff;
    vfs_fstat(fd, &statbuff);

    tiny_exec_hdr_t header;
    int count = vfs_read(fd, &header, sizeof(tiny_exec_hdr_t));
    if (count < 0) {
        vfs_close(fd);
        return count;
    }

    kdbg("exec: magic=%s, arch=%d, break=0x%x, stack=%d, entry=0x%x\n", header.magic, header.arch, header.program_break, header.stack_size, header.entry_point);
    
    if (strncmp(header.magic, TINY_EXEC_MAGIC, 4) != 0) goto fmt_error;
    if (header.arch != ARMV8_M_MAIN) goto fmt_error;
    if (TINY_EXEC_OSABI_MAJOR(header.os_abi) != 1) goto fmt_error;
    
    uint8_t* program_base = page_alloc(header.program_break);
    if (!program_base) {
        vfs_close(fd);
        return -ENOMEM;
    }

    kdbg("base=0x%x\n", (uint32_t) program_base);
    
    vfs_lseek(fd, 0, 0);
    count = vfs_read(fd, program_base, header.program_break);
    kdbg("bytes_loaded=%d\n", count);
    vfs_close(fd);
        
    //pushing argv to the user stack
    uint8_t* user_stack = program_base + (header.program_break - header.stack_size);
    
    process_desc_t new = {
        .user_stack = user_stack,
        .size = header.stack_size,
        .entry_point = (void (*)(void)) program_base + header.entry_point,
        .argv = argv,
        .stopped = 1,
        .kernel_mode = 0
    };

    struct proc* p = proc_create(&new);
    if (!p) {
        page_free(program_base);
        return -EAGAIN;
    }


    p->ppid = current_process->pid;
    p->pgrp = current_process->pgrp;
    p->sigmask = current_process->sigmask | BLOCK_MASK;
    p->program_base = program_base;
    p->program_size = header.program_break;
    
    //inherit current working directory
    mutex_lock(&vfs_cache_lock);
    struct inode* cwd = current_process->cwd;
    cwd->refcount++;
    p->cwd = cwd;
    mutex_unlock(&vfs_cache_lock);

    //inherit the credentials of the previous process
    p->credentials = current_process->credentials;
    
    if (statbuff.st_mode & S_ISUID) {
        p->credentials.euid = statbuff.st_uid;
    }
    if (statbuff.st_mode & S_ISGID) {
        p->credentials.egid = statbuff.st_gid;
    }

    for (int i = 0; i < FD_SETSIZE; i++) {
        if (FD_ISSET(i, (*fd_list))) {
            struct file* f = proc_fd_get(current_process, i);
            if (!f) continue;
            f->refcount++;
            proc_fd_add(p, f);
        }
    }

    //proc_unblock_process(p);
    
    kinfo("exec: loaded %s, pid=%d, ppid=%d\n", path, p->pid, p->ppid);

    return p->pid;

fmt_error:
    vfs_close(fd);
    return -ENOEXEC;
}

pid_t sys_getpdid(int type)
{
    if (type == 0) return current_process->pid;
    if (type == 1) return current_process->ppid;
    return 0;
}

int sys_setpgrp(pid_t pid, pid_t pgid)
{
    if (pgid < 0) return -EINVAL;
    if (pid == 0) pid = current_process->pid;
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    if (pgid == 0) pgid = p->pid;

    cred_t cred = p->credentials;
    if (!check_owner(cred)) {
        proc_free_process(p);
        return -EPERM;
    }
    
    p->pgrp = pgid;
    proc_free_process(p);
    return 0;
}

pid_t sys_getpgrp(pid_t pid)
{
    if (pid == 0) pid = current_process->pid;
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    pid_t pgrp = p->pgrp;
    proc_free_process(p);
    return pgrp;
}

int sys_kill(pid_t pid, int sig)
{
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    
    if (check_owner(p->credentials)) {
        proc_free_process(p);
        return proc_kill(pid, sig);
    }
    proc_free_process(p);
    return -EPERM;
}

int sys_sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
    sigset_t current_set = current_process->sigmask;
    if (oldset) *oldset = current_set;
    if (!set) return 0;
    sigset_t new_set = *set;

    if (how == SIG_BLOCK) {
        current_set &= ~(new_set & ~BLOCK_MASK);
    } else if (how == SIG_UNBLOCK) {
        current_set |= new_set;
    } else if (how == SIG_SETMASK) {
        current_set = new_set | BLOCK_MASK;
    } else {
        return -EINVAL;
    }

    current_process->sigmask = current_set;
    return 0;
}

void sys_exit(int return_code)
{
    proc_stop_scheduling();
    proc_mark_zombie(current_process, return_code);
    proc_kill(current_process->ppid, SIGCHLD);

    proc_schedule(); //in case proc_kill doesnt trigger the scheduler
}

pid_t sys_waitpid(pid_t pid, int* wstatus, int options)
{
    
    mutex_lock(&proc_acces_lock);
    
    int child_count = 0;
    struct proc* p = proc_active_list;
    while(p) {
        if (p->ppid == current_process->pid) {
            child_count++;
            pid_t p_pid = p->pid;
            if (p->state == PROC_ZOMBIE && (p_pid == pid || pid == -1)) {
                mutex_unlock(&proc_acces_lock);
                int exit_code = proc_reap(p);
                if (wstatus) *wstatus = exit_code;
                current_process->exit_code = 0; //clear signal
                return p_pid;
            }
        }
        p = p->next;
    }

    mutex_unlock(&proc_acces_lock);

    if (options == WNOHANG) {
        return -ECHILD;
    }
    //if we dont have any children we cant wait
    if (child_count == 0) {
        return -ECHILD;
    }
    proc_block(current_process);
    current_process->exit_code = 0;
    mutex_lock(&proc_acces_lock);
    p = proc_active_list;
    while(p) {
        if (p->ppid == current_process->pid) {
            pid_t p_pid = p->pid;
            if (p->state == PROC_ZOMBIE && (p_pid == pid || pid == -1)) {
                mutex_unlock(&proc_acces_lock);
                int exit_code = proc_reap(p);
                if (wstatus) *wstatus = exit_code;
                return p_pid;
            }
        }
        p = p->next;
    }
    mutex_unlock(&proc_acces_lock);
    
    return -ECHILD;
}


