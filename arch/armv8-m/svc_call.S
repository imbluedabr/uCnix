
    .syntax unified
    .text
    .thumb
    .extern current_process
    .extern proc_sched_new_task
    .extern proc_sched_started
    .extern syscall_thread_main

/*
    so the way i handle syscalls is by doing the minimum amount of work inside the svc handler to improve determinism and latency of tasks.
    each process has 2 modes, user mode and kernel mode, each mode has seperate stacks, when an svc happens i just switch the current mode, so it would look something like this:
    user (SVC) -> kernel (SVC) -> user

    this way the kernel mode "replaces" the current user thread context with its own context and then returns into it, this way the actual syscall code runs in thread mode and this way the actuall context switcher in pendsv doesnt even have to know about the different modes or do complicated stuff like preempting the actual svcall_handler in handler mode
*/

    .thumb_func
    .align 2
    .globl   svc_handler
    .type    svc_handler, %function
svc_handler:
    
    /* if the scheduler is not started we must start it*/
    ldr r3, =proc_sched_started
    ldrb r0, [r3]
    cbz r0, initial_process_entry

    /* now we store the current context */
    mrs r1, psp
    isb

    ldr r0, =current_process
    ldr r2, [r0]
    stmdb r1!, {r4-r11, r14}
    str r1, [r2]            /* Store the psp in proc->psp*/
    
    /* swap current psp, control and splim with save_psp, save_control and save_splim */
    mov r0, r2
    ldmia r0, {r1-r6}
    stmia r0!, {r4-r6}
    stmia r0!, {r1-r3}
    sub r0, r0, #24
    

    /* restore the other process mode */
    ldr r0, =current_process
    ldr r2, [r0]
    ldr r1, [r2]            /* read the psp from the current process */
    ldmia r1!, {r4-r11, r14}
    msr psp, r1
    isb

    ldr r1, [r2, #8]        /* load proc->splim into psplim */
    msr psplim, r1
    isb

    ldr r1, [r2, #4]        /* load proc->control into control*/
    msr CONTROL, r1         /* this sets the privilege and stack bit */
    isb

    bx lr                   /* return back to the new process*/

initial_process_entry: /* we use this to run the first task and bootstrap the scheduler */
    /* set proc_sched_started to true */
    movs r0, #1
    strb r0, [r3]

    ldr r3, =current_process
    ldr r2, [r3]
    ldr r0, [r2]            /* read the psp from the current process */
    ldmia r0!, {r4-r11, r14}
    msr psp, r0
    isb

    ldr r0, [r2, #8]        /* load proc->splim into psplim */
    msr psplim, r0
    isb

    ldr r0, [r2, #4]        /* load proc->control into control*/
    msr CONTROL, r0         /* this sets the privilege and stack bit */
    isb

    bx lr                   /* return back to the new process*/






/* this is the syscall thread trampoline */
    .thumb_func
    .align 2
    .globl   svc_thread
    .type    svc_thread, %function
svc_thread:
    
    bl syscall_thread_main
    /* switch back to usermode */
    svc #0

    b svc_thread

















