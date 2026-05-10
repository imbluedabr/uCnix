
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
    mrs r0, psp
    /* pass pc to panic_handler */
    /* frame->r0 = frame->pc*/
    ldr r1, [r0, #24]
    str r1, [r0, #0]
    /* frame->pc = panic_handler */
    ldr r1, =panic_handler
    str r1, [r0, #24]

    /* return to thread mode into the panic handler */
    ldr r0, =0xFFFFFFB8
    mov lr, r0
    bx lr


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
    bx lr

