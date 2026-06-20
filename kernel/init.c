#include <lib/kprint.h>
#include <kernel/proc.h>
#include <kernel/device.h>
#include <kernel/majors.h>
#include <kernel/settings.h>
#include <kernel/time.h>
#include <kernel/board.h>
#include <kernel/uname.h>
#include <kernel/alloc.h>
#include <kernel/interrupt.h>
#include <kernel/syscall.h>
#include <kernel/devtbl.h>
#include <kernel/exec.h>
#include <kernel/page.h>
#include <fs/vfs.h>
#include <drivers/tty.h>
#include <drivers/hd44xxx.h>
#include <drivers/romdisk.h>
#include <drivers/usart.h>
#include <lib/kmalloc.h>
#include <lib/stdlib.h>
#include <uapi/sys/fcntl.h>
#include <uapi/sys/disk.h>
#include <uapi/sys/dir.h>
#include <uapi/signal.h>

void kernel_worker_process()
{
    kinfo("init: kernel worker started\n");
    time_t last = get_kernel_ticks();
    while (1) {
        if ((get_kernel_ticks() - last) > 1000) {
            last = get_kernel_ticks();
            system_blink();
        }
        if (current_process->exit_code != 0) {
            proc_stop_scheduling();
            
            if (current_process->exit_code == SIGCHLD) {

                struct proc* p = proc_active_list;
                while(p) {
                    if (p->state == PROC_ZOMBIE) {
                        proc_reap(p);
                        if (p->pid == 1) goto stop;
                        break;
                    }
                    p = p->next;
                }
            }
            current_process->exit_code = 0;
            
            proc_schedule();
        }
        device_global_update();
    }

stop:
    kerr("init process exited!\n");
    struct memstat buff;
    heap_stat(&kernel_allocator, &buff);
    kdbg("heap: blocks_used=%d, blocks_total=%d, bytes_used=%d, bytes_total=%d, frag=%d\n", buff.blocks_used, buff.blocks_total, buff.bytes_used, buff.bytes_total, buff.fragmentation);
    
    inode_stat();

    kinfo("halting kernel...\n");
    //proc_stop_scheduling();
    while (1) {
        __WFI();
    }
}

void kernel_init_process()
{
    kinfo("vfs: mounting rootfs on dev (%d,%d) of type (%s) on /\n", MAJOR(ROOTFS_DEVNO), MINOR(ROOTFS_DEVNO), ROOTFS_TYPE);

    int status = vfs_mount_root(ROOTFS_DEVNO, ROOTFS_TYPE, 0);
    if (status < 0) {
        kerr("vfs: rootfs mount failed! errno=%d\n", status);
        goto abort;
    }
    struct inode* root_cwd = mount_table[0].root;
    struct filesystem* fs = root_cwd->fs;
    root_cwd->refcount++;
    current_process->cwd = root_cwd;
    kdbg("vfs: rootfs: block_count=%d, block_size=%d, block_used=%d\n", fs->block_count, fs->block_size, fs->block_used);
    

    kinfo("vfs: mounting devfs on /dev\n");
    status = vfs_mount_dev("/dev", "devfs", 0);
    if (status < 0) {
        kerr("vfs: devfs mount failed! errno=%d\n", status);
        goto abort;
    }

    
    struct filesystem* devfs = mount_table[1].root->fs;
    devfs->fops->mknod(devfs, "tty0", FS_MAKE_PERM(0, 0, 0666), MKDEV(TTY_MAJOR, 0));
    devfs->fops->mknod(devfs, "tty1", FS_MAKE_PERM(0, 0, 0666), MKDEV(TTY_MAJOR, 1));
    
    
    
    int stdin = vfs_open("/dev/tty0", O_RDWR);
    int stdout = vfs_fcntl(stdin, F_DUPFD, 0);
    int stderr = vfs_fcntl(stdin, F_DUPFD, 0);
    kinfo("init: starting userspace init\n");
    fd_set fd_list;
    FD_ZERO(fd_list);
    FD_SET(stdin, fd_list);
    FD_SET(stdout, fd_list);
    FD_SET(stderr, fd_list);
    status = sys_spawn(INIT_PATH, &fd_list, NULL);
    
    //reparent userspace init
    int irq = disable_interrupts();
    struct proc* p = proc_get_process(2);
    if (!p) goto abort;
    p->pid = 1;
    p->ppid = 0;
    current_process->pid = 2;
    current_process->ppid = 0;
    enable_interrupts(irq);
    proc_free_process(p);
    proc_kill(1, SIGCONT);
    
abort:
    sys_exit(0);
}

void kernel_pre_init()
{
    system_init(); //initialize cache, flash and other basic board specific stuff
    interrupt_init(); //load the ram vector table
    kmalloc_init(__heap_start, __heap_size); //initialize the heap
    page_init();
    time_init();
    device_init();
    proc_init();
    syscall_init();
    vfs_init();

    usart_init();
    hd44xxx_init();
    romdisk_init();
    tty_init();
}

const process_desc_t kernel_worker_proc = {
    .entry_point = &kernel_worker_process,
    .argv = NULL,
    .stopped = 0,
    .kernel_mode = 1
};
const process_desc_t kernel_init_proc = {
    .entry_point = &kernel_init_process,
    .argv = NULL,
    .stopped = 0,
    .kernel_mode = 1
};


[[noreturn]] void kernel_init()
{
    //initialize boot console
    dev_t tty0;
    dev_t tty1;

    boot_console = device_create(&tty0, TTY_MAJOR, &(struct tty_desc) {
            .reader = INIT_CONSOLE_RDEV,
            .writer = INIT_CONSOLE_WDEV
            });
       
    kprintf("\e[1;35m%s %s %s %s %s\n\e[1;39m", local_uname.sysname, local_uname.nodename, local_uname.release, local_uname.version, local_uname.machine);
    
    devtbl_init();

    device_create(&tty1, TTY_MAJOR, &(struct tty_desc) {
        .reader = MKDEV(USART_MAJOR, 1),
        .writer = MKDEV(USART_MAJOR, 1)
        });

    kinfo("init: starting kernel worker\n");
    struct proc* p = proc_create(&kernel_worker_proc);
    p->sigmask = 1 << SIGCHLD;
    kinfo("init: starting kernel init\n");
    p = proc_create(&kernel_init_proc);
    p->sigmask = (1 << SIGCHLD) | (1 << SIGCONT); //enable sigchld signal

    proc_start_scheduling();
    while(1);
}


