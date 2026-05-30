#include <lib/kprint.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>

//stack protector lol
[[gnu::used]] uint32_t __stack_chk_guard = 0xDEADBEEF;

[[gnu::used]] const char __stack_chk_fail_msg[] = "stack check fail!";



