
    .text
    .thumb
    .extern panic_handler

    .thumb_func
    .align 2
    .globl   nmi_handler
    .type    nmi_handler, %function
nmi_handler:
    bkpt


    .thumb_func
    .align 2
    .globl   hardfault_handler
    .type    hardfault_handler, %function
hardfault_handler:
    /* create emergency stack */
    ldr   r1, =__stack_top
    sub   r1, r1, #32           /* allocate the stack frame */
    msr   msp, r1
    dsb

    /* panic_fram->r0 = frame->pc */
    ldr   r2, [r0, #24]
    str   r2, [r1, #0]         @ new_frame->r0

    /* zero other regs */
    movs  r2, #0
    str   r2, [r1, #4]
    str   r2, [r1, #8]
    str   r2, [r1, #12]
    str   r2, [r1, #16]
    str   r2, [r1, #20]

    /* panic_frame->pc = &panic_handler */
    ldr   r2, =panic_handler
    str   r2, [r1, #24]

    /* panic_frame->cpsr = 0x01000000 so thumb bit is set */
    ldr   r2, =0x1000000
    str   r2, [r1, #28]

    ldr   r0, =0xFFFFFFB8
    mov   lr, r0
    bx    lr

    .thumb_func
    .align 2
    .globl   mpufault_handler
    .type    mpufault_handler, %function
mpufault_handler:
    bkpt

    .thumb_func
    .align 2
    .globl   busfault_handler
    .type    busfault_handler, %function
busfault_handler:
    bkpt

    .thumb_func
    .align 2
    .globl   usagefault_handler
    .type    usagefault_handler, %function
usagefault_handler:
    bkpt

    .thumb_func
    .align 2
    .globl   debugevent_handler
    .type    debugevent_handler, %function
debugevent_handler:
    bkpt
    bx lr

