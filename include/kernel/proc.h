#pragma once
#include <stdint.h>

typedef uint8_t pid_t;

struct proc {
#ifdef __ARM_ARCH_8M_MAIN__
    uint8_t* psp;
    uint32_t control;
    uint8_t* splim;
    uint32_t entry_point;
#endif
};

extern struct proc* current_process;
#define PROC_TABLE_LEN 4

void proc_init();
struct proc* proc_create(uint8_t* stack, uint32_t size, void (*entry_point)());

//starts the scheduler and never returns
void proc_start_scheduling();

