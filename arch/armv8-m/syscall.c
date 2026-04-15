#include <kernel/syscall.h>
#include <kernel/interrupt.h>
#include <board/board.h>
#include <stddef.h>

extern void svc_handler();

void syscall_init()
{
    register_interrupt(SVCall_IRQn, NULL, &svc_handler);
}


