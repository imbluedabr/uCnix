    .syntax unified
    .text
    .thumb
    .extern current_process
    .extern proc_sched_new_task




    .thumb_func
    .align 2
    .globl   pendsv_handler
    .type    pendsv_handler, %function
pendsv_handler:
    /* bkpt */
    mrs r0, psp
    isb

    ldr r3, =current_process
    ldr r2, [r3]
    stmdb r0!, {r4-r11, r14}
    str r0, [r2]            /* Store the psp in proc->psp*/
    stmdb sp!, {r3}
    
    cpsid i
    bl proc_sched_new_task /* schedule a new task */
    cpsie i

    ldmia sp!, {r3}

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




