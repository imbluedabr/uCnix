#include <kernel/exec.h>
#include <kernel/tiny_exec.h>
#include <kernel/proc.h>
#include <kernel/page.h>
#include <fs/vfs.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>

#include <uapi/sys/fcntl.h>
#include <uapi/sys/errno.h>
#include <stdint.h>

int sys_spawn(const char* path)
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
        .stopped = 0,
        .kernel_mode = 0
    };

    proc_create(&new);
    
    return 0;

fmt_error:
    vfs_close(fd);
    return -ENOEXEC;
}


