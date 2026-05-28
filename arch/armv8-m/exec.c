#include <kernel/exec.h>
#include <kernel/tiny_exec.h>
#include <kernel/proc.h>
#include <fs/vfs.h>
#include <lib/kprint.h>
#include <lib/stdlib.h>

#include <uapi/sys/fcntl.h>
#include <uapi/sys/errno.h>

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
    
    

    return 0;

fmt_error:
    vfs_close(fd);
    return -ENOEXEC;
}


