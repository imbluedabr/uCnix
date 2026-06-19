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
    /* bkpt */
    /* sp should point to the start of the argv array, so it should be passed via r0 */
    movs r0, #0
    mov r1, sp

loop:
    ldr r2, [r1]
    cmp r2, #0
    beq start_main
    add r1, #4
    add r0, #1
    b loop

start_main:
    mov r1, sp
    bl main
    svc #41
    b .

