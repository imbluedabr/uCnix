#include <lib/kprint.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>

//stack protector lol
uint32_t __stack_chk_guard = 0xDEADBEEF;

const char __stack_chk_fail_msg[] = "stack check fail!";



