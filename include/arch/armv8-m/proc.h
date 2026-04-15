#pragma once
#include <stdint.h>
#include <board/board.h>

struct context_frame {
    uint32_t callee_regs[8];
    uint32_t exc_return; //link register used to return to the process
    uint32_t caller_regs[5]; //r0-r3, r12
    uint32_t process_lr; //linker register before the preemption
    uint32_t pc; //program counter
    xPSR_Type cpsr; //combined program status register
};

void pendsv_handler();
void proc_systick_handler();
void proc_bootstrap();
void proc_sched_new_task();


