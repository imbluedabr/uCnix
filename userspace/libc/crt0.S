    .syntax unified
    .arch armv8-m.main

.section .header
    /* magic value */
    .byte 'T'
    .byte 'E'
    .byte 'X'
    .byte 0x00
    /* architecture e.g armv8-m.mainline */
    .byte 0x01
    /* os abi */
    .byte 0x10
    /* padding */
    .zero 0x2

    /* program break */
    .word __stack_top
    /* stack size */
    .word __stack_size
    /* entry point */
    .word _start
    /* padding */
    .zero 0x4

    .text
    .thumb
    .extern main
    .globl _start

    .align 2
_start:
    bl main
    svc #41
    b .

