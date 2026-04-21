#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <board/board.h>

struct exception_frame {
    uint32_t caller_regs[5];
    uint32_t lr;
    uint32_t pc;
    xPSR_Type cpsr;
};

struct context_frame {
    uint32_t callee_regs[8];
    uint32_t exc_return; //link register used to return to the process
    struct exception_frame base_frame;
};

extern bool proc_sched_started;

void pendsv_handler();
void proc_bootstrap();
void proc_sched_new_task();


