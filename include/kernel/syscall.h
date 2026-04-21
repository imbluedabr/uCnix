#pragma once

#define SYSCALL_COUNT 2
typedef enum {
    SYS_WRITE,
    SYS_RET
} syscall_e;


void syscall_init();

