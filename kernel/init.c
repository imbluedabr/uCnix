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
#include <fs/vfs.h>
#include <drivers/tty.h>
#include <drivers/hd44xxx.h>
#include <drivers/romdisk.h>
#include <drivers/usart.h>
#include <uapi/sys/fcntl.h>
#include <uapi/sys/disk.h>
#include <uapi/sys/dir.h>

[[gnu::aligned(8)]] uint8_t kernel_init_stack[256];
[[gnu::aligned(8)]] uint8_t kernel_worker_stack[256];

void kernel_worker_process()
{
    kinfo("init: kernel worker started\n");
    time_t last = get_kernel_ticks();
    while (1) {
        if ((get_kernel_ticks() - last) > 1000) {
            last = get_kernel_ticks();
            system_blink();
        }
        device_global_update();
    }
}

void kernel_init_process()
{
    kinfo("vfs: mounting rootfs on dev (%d,%d) of type (%s) on /\n", MAJOR(ROOTFS_DEVNO), MINOR(ROOTFS_DEVNO), ROOTFS_TYPE);

    
    int status = vfs_mount_root(ROOTFS_DEVNO, ROOTFS_TYPE, 0);
    if (status < 0) {
        kerr("vfs: rootfs mount failed! errno=%d\n", status);
    } else {
        struct superblock* fs = vfs_root.fs;
        kdbg("vfs: rootfs: block_count=%d, block_size=%d, block_used=%d\n", fs->block_count, fs->block_size, fs->block_used);
    }
    

    kinfo("halting kernel...\n");
    proc_stop_scheduling();
    enable_interrupts(0); //hack
    while (1) {
    }
}

void kernel_pre_init()
{
    system_init(); //initialize cache, flash and other basic board specific stuff
    interrupt_init(); //load the ram vector table
    init_heap(__heap_start, 2048); //initialize the heap
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

[[noreturn]] void kernel_init()
{
    //initialize boot console
    dev_t tty0;
    
    boot_console = device_create(&tty0, TTY_MAJOR, &(struct tty_desc) {
            .reader = INIT_CONSOLE_RDEV,
            .writer = INIT_CONSOLE_WDEV
            });

    kprintf("%s %s %s %s\n", uname.sysname, uname.release, uname.version, uname.machine);
    
    devtbl_init();

    kinfo("init: starting kernel worker\n");
    proc_create(kernel_worker_stack, NULL, 256, &kernel_worker_process);
    kinfo("init: starting kernel init\n");
    proc_create(kernel_init_stack, NULL, 256, &kernel_init_process);
    
    proc_start_scheduling();
    while(1);
}


