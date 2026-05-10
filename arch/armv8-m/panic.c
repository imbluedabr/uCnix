#include <kernel/proc.h>
#include <lib/kprint.h>
#include <kernel/board.h>
#include <arch/armv8-m/proc.h>

union CFSR {
    struct {
        struct {
            uint8_t IACCVIOL    : 1;
            uint8_t DACCVIOL    : 1;
            uint8_t res0        : 1;
            uint8_t MUNSTKERR   : 1;
            uint8_t MSTKERR     : 1;
            uint8_t MLSPERR     : 1;
            uint8_t res1        : 1;
            uint8_t MMARVALID   : 1;
        } MMFSR;
        struct {
            uint8_t IBUSERR     : 1;
            uint8_t PRECISERR   : 1;
            uint8_t res0        : 1;
            uint8_t UNSTKERR    : 1;
            uint8_t STKERR      : 1;
            uint8_t LSPERR      : 1;
            uint8_t res1        : 1;
            uint8_t BFARVALID   : 1;
        } BFSR;
        struct {
            uint16_t UNDEFINSTR : 1;
            uint16_t INVSTATE   : 1;
            uint16_t INVPC      : 1;
            uint16_t NOCP       : 1;
            uint16_t STKOF      : 1;
            uint16_t res0       : 3;
            uint16_t UNALIGNED  : 1;
            uint16_t DIVBYZERO  : 1;
        } UFSR;
    } b;
    uint32_t raw;
};

void thread_panic(const char* msg)
{
    __disable_irq();
    uint32_t lr;
    __asm volatile ("mov %0, lr" : "=r"(lr));
    struct proc* p = proc_stop_scheduling();
    __enable_irq();
    kprintf("\nPANIC!!! - %s\n", msg);
    kprintf("PID=%d, PPID=%d, PSP=0x%x, PC=0x%x\n", p->pid, p->ppid, p->psp, lr);
    while(1) {
        __WFI();
    }
    
}

void panic_handler(uint32_t pc)
{
    __asm volatile ("cpsid i");
    /*
    uint32_t lr;
    __asm volatile ("mov %0, lr" : "=r"(lr));*/

    struct proc* p = proc_stop_scheduling();
    
    console_lock.count = 0;
    console_lock.lock = 0;
    __enable_irq();

    kprintf("\nPANIC!!! - a critical exception occured\n");

    if (SCB->HFSR & (1 << 30)) { //check if the FORCED bit is set
        kputs("While processing an exception another exception occured.\n");
    }
    kprintf("\nproc: PID=%d, PPID=%d, PSP=0x%x, PC=0x%x\n\n",p->pid, p->ppid, p->psp, pc);

    union CFSR cfsr;
    cfsr.raw = SCB->CFSR;
    
    if ((cfsr.raw & 0xFF) != 0) { //memmanage fault
        kprintf("MemManage Fault occured\nMMFSR: 0x%x\n", (uint32_t) cfsr.raw & 0xFF);
        if (cfsr.b.MMFSR.MMARVALID) {
            kprintf("MMFAR: 0x%x\n", SCB->MMFAR);
        }
    }
    if ((cfsr.raw & 0xFF00) != 0) { //busfault
        kprintf("BusFault occured\nBFSR: 0x%x\n", (uint32_t) (cfsr.raw & 0xFF00) >> 8);
        if (cfsr.b.BFSR.BFARVALID) {
            kprintf("BFAR: 0x%x\n", SCB->BFAR);
        }
    }
    if ((cfsr.raw & 0xFFFF0000) != 0) {
        kprintf("UsageFault occured\nUFSR: 0x%x\n", (uint32_t) (cfsr.raw & 0xFFFF0000) >> 16);
    }

    while(1) {
        __WFI();
    }
}


