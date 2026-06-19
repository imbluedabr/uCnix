#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <kernel/proc.h>
#include <kernel/exec.h>
#include <kernel/sysctl.h>
#include <kernel/uname.h>
#include <fs/vfs.h>

#include <board/board.h>
#include <arch/armv8-m/proc.h>
#include <arch/armv8-m/syscall.h>
#include <uapi/syscalls.h>
#include <uapi/sys/errno.h>
#include <lib/kprint.h>
#include <stddef.h>

typedef void (*syscall_t)(struct exception_frame*);

static inline int check_uptr(void* uptr)
{
    if ((uint8_t*) uptr < current_process->program_base) return -EFAULT;
    if ((uint8_t*) uptr > (current_process->program_base + current_process->program_size)) return -EFAULT;
    return 0;
}

static void s_read(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_read(f->caller_regs[0], (void*) f->caller_regs[1], f->caller_regs[2]);
}

static void s_write(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_write(f->caller_regs[0], (void*) f->caller_regs[1], f->caller_regs[2]);
}

static void s_open(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_open((const char*) f->caller_regs[0], f->caller_regs[1]);
}

static void s_close(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_close(f->caller_regs[0]);
}

static void s_fstat(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_fstat(f->caller_regs[0], (struct stat*) f->caller_regs[1]);
}

static void s_lseek(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_lseek(f->caller_regs[0], f->caller_regs[1], f->caller_regs[2]);
}

static void s_ioctl(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_ioctl(f->caller_regs[0], f->caller_regs[1], (void*) f->caller_regs[2]);
}

static void s_access(struct exception_frame* f)
{
    
}

static void s_select(struct exception_frame* f)
{

}

static void s_fcntl(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_fcntl(f->caller_regs[0], f->caller_regs[1], f->caller_regs[2]);
}

static void s_chdir(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_chdir((const char*) f->caller_regs[0]);
}

static void s_mkdir(struct exception_frame* f)
{
    
}

static void s_rmdir(struct exception_frame* f)
{

}

static void s_readdir(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_readdir(f->caller_regs[0], (struct dirent*) f->caller_regs[1], f->caller_regs[2]);
}

static void s_link(struct exception_frame* f)
{

}

static void s_unlink(struct exception_frame* f)
{

}

static void s_mknod(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_mknod((const char*) f->caller_regs[0], f->caller_regs[1], f->caller_regs[2]);
}

static void s_fstatfs(struct exception_frame* f)
{
    f->caller_regs[0] = vfs_fstatfs(f->caller_regs[0], (struct statfs*) f->caller_regs[1]);
}

static void s_fchmod(struct exception_frame* f)
{
    
}

static void s_fchown(struct exception_frame* f)
{

}

static void s_umask(struct exception_frame* f)
{

}

static void s_sync(struct exception_frame* f)
{

}

static void s_mount(struct exception_frame* f)
{

}

static void s_umount(struct exception_frame* f)
{

}

static void s_getpdid(struct exception_frame* f) {
    f->caller_regs[0] = sys_getpdid(f->caller_regs[0]);
}

static void s_spawn(struct exception_frame* f) {
    f->caller_regs[0] = sys_spawn((const char*) f->caller_regs[0], (fd_set*) f->caller_regs[1], (const char**) f->caller_regs[2]);
}

static void s_exit(struct exception_frame* f) {
    sys_exit(f->caller_regs[0]);
}

static void s_waitpid(struct exception_frame* f)
{
    f->caller_regs[0] = sys_waitpid(f->caller_regs[0], (int*) f->caller_regs[1], f->caller_regs[2]);
}

static void s_kill(struct exception_frame* f)
{
    f->caller_regs[0] = sys_kill(f->caller_regs[0], f->caller_regs[1]);
}

static void s_sigprocmask(struct exception_frame* f)
{
    f->caller_regs[0] = sys_sigprocmask(f->caller_regs[0], (const sigset_t*) f->caller_regs[1], (sigset_t*) f->caller_regs[2]);
}

static void s_sysctl(struct exception_frame* f)
{
    f->caller_regs[0] = sys_sysctl(f->caller_regs[0], (void*) f->caller_regs[1], f->caller_regs[2]);
}

static void s_uname(struct exception_frame* f)
{
    f->caller_regs[0] = sys_uname((struct utsname*) f->caller_regs[0]);
}

static const syscall_t s_table[SYSCALL_COUNT] = {
    [SYS_READ] = s_read,
    [SYS_WRITE] = s_write,
    [SYS_OPEN] = s_open,
    [SYS_CLOSE] = s_close,
    [SYS_FSTAT] = s_fstat,
    [SYS_LSEEK] = s_lseek,
    [SYS_IOCTL] = s_ioctl,
    [SYS_ACCESS] = s_access,
    [SYS_SELECT] = s_select,
    [SYS_FCNTL] = s_fcntl,
    [SYS_CHDIR] = s_chdir,
    [SYS_MKDIR] = s_mkdir,
    [SYS_RMDIR] = s_rmdir,
    [SYS_READDIR] = s_readdir,
    [SYS_LINK] = s_link,
    [SYS_UNLINK] = s_unlink,
    [SYS_MKNOD] = s_mknod,
    [SYS_FSTATFS] = s_fstatfs,
    [SYS_FCHMOD] = s_fchmod,
    [SYS_FCHOWN] = s_fchown,
    [SYS_UMASK] = s_umask,
    [SYS_SYNC] = s_sync,
    [SYS_MOUNT] = s_mount,
    [SYS_UMOUNT] = s_umount,

    [SYS_GETPDID] = s_getpdid,
    [SYS_SPAWN] = s_spawn,
    [SYS_EXIT] = s_exit,
    [SYS_WAITPID] = s_waitpid,
    [SYS_KILL] = s_kill,
    [SYS_SIGPROCMASK] = s_sigprocmask,

    [SYS_SYSCTL] = s_sysctl,
    [SYS_UNAME] = s_uname
};

/*  Syscalls:
 *      so syscalls have to be preemptible but i dont want to deal with nested interrupt preemption,
 *      so i modify the pc in the stack frame to point to the syscall function and switch to privileged mode.
 *      this way the process will basicly switch to kernel mode and execute the syscall code which now can be preempted since its running in thread mode.
 *      now when the syscall returns it calls svc which returns from a syscall and returns to the user process.
 *      and then we only need one kernel worker for pending functions like driver update functions or other small stuff.
*/

void syscall_init()
{
    register_interrupt(SVCall_IRQn, NULL, &svc_handler);
    NVIC_SetPriority(SVCall_IRQn, 7);
}


void syscall_thread_main()
{
    struct context_frame* frame = (struct context_frame*) current_process->save_psp;
    uint8_t svcno = *((uint8_t*) frame->base_frame.pc - 2);
    //kdbg("svc:, %d, pid: %d\n", svcno, current_process->pid);
   
    syscall_t func = s_table[svcno];
    if (!func) {
        frame->base_frame.caller_regs[0] = -ENOSYS;
    } else {
        func(&frame->base_frame);
    }
}





