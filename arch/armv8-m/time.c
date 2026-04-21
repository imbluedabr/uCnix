#include <kernel/time.h>
#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <arch/armv8-m/proc.h>
#include <stddef.h>

volatile uint32_t time_ticks;

void systick_handler()
{
    time_ticks++;
    if (current_process != NULL) {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        __DSB();
    }
}

uint32_t get_kernel_ticks()
{
    return time_ticks;
}

void simple_delay(uint32_t ticks)
{
    uint32_t curr = get_kernel_ticks();
    while((get_kernel_ticks() - curr) < ticks);
}

void time_init()
{
    __disable_irq();
    time_ticks = 0;
    register_interrupt(SysTick_IRQn, NULL, systick_handler);
    NVIC_SetPriority(SysTick_IRQn, 3);
    __enable_irq();
}


