
    .text
    .thumb

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
    bkpt

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

