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
#include <uapi/signal.h>
#include <stdint.h>

int sys_spawn(const char* path, fd_set fd_list)
{
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) return fd;

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

    kdbg("base=0x%x\n", (uint32_t) program_base);
    
    vfs_lseek(fd, 0, 0);
    count = vfs_read(fd, program_base, header.program_break);
    kdbg("bytes_loaded=%d\n", count);
    vfs_close(fd);

    
    process_desc_t new = {
        .user_stack = program_base + (header.program_break - header.stack_size),
        .size = header.stack_size,
        .entry_point = (void (*)(void)) program_base + header.entry_point,
        .stopped = 1,
        .kernel_mode = 0
    };

    struct proc* p = proc_create(&new);
    p->ppid = current_process->pid;
    
    for (int i = 0; i < FD_SETSIZE; i++) {
        if (FD_ISSET(i, fd_list)) {
            struct file* f = proc_fd_get(current_process, i);
            if (!f) continue;
            f->refcount++;
            proc_fd_add(p, f);
        }
    }

    proc_unblock_process(p);

    return p->pid;

fmt_error:
    vfs_close(fd);
    return -ENOEXEC;
}

int sys_kill(pid_t pid, int sig)
{
    struct proc* p = proc_get_process(pid);
    if (!p) return -ESRCH;
    
    if (current_process->credentials.euid == p->credentials.euid || current_process->credentials.egid == p->credentials.egid) {
        return proc_kill(pid, sig);
    }
    return -EPERM;
}

void sys_exit(int return_code)
{
    disable_interrupts();
    proc_mark_zombie(current_process, return_code);
    proc_kill(current_process->ppid, SIGCHLD);
}

pid_t sys_waitpid(pid_t pid, int* wstatus, int options)
{
    proc_block(current_process);
    kputs("wow");
    struct proc* p = proc_active_list;
    while(p) {
        if (p->pid == pid) {
            *wstatus = proc_reap(p);
            return pid;
        }
    }
    *wstatus = -1;
    return 0;
}


