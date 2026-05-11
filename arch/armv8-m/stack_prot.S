
    .syntax unified
    .text
    .thumb
    .extern thread_panic
    .extern __stack_chk_fail_msg

    .thumb_func
    .align 2
    .globl   __stack_chk_fail
    .type    __stack_chk_fail, %function
__stack_chk_fail:
    ldr r0, =__stack_chk_fail_msg
    b thread_panic


