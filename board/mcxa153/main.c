#include <stddef.h>
#include <board/mcxa153/board.h>
#include <kernel/interrupt.h>
#include <kernel/alloc.h>
#include <kernel/device.h>
#include <uapi/majors.h>
#include <drivers/usart.h>


void gpio_output_init(void)
{
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_PORT3(1);

    PORT3->PCR[13] = 0x00008000;
    
    GPIO3->PDOR |= (1<<13);

    GPIO3->PDDR |= (1<<13);
}

static volatile uint32_t ticks = 0;

void systick_handler()
{
    ticks++;
}

[[gnu::aligned(8)]] char heap[2048];

void main()
{
    system_init(); //initialize cache, flash and other basic board specific stuff
    interrupt_init(); //load the ram vector table
    init_heap(&heap, 2048); //initialize the heap
    device_init();
    usart_init();

    gpio_output_init();
   
    SysTick_Config(48000);
    
    register_interrupt(SysTick_IRQn, NULL, systick_handler);
    
    dev_t usart0_devno;
    struct device* usart0 = device_create(&usart0_devno, USART_MAJOR, &(struct usart_desc) {
            .base = LPUART0,
            .baud = 5, //baud rate is 9600
            .irq = LPUART0_IRQn,
            .type = MCXA_LPUART
            });
    
    __enable_irq();
    //__BKPT();
    while(1) {
        if (ticks > 1000) {
            ticks = 0;
            GPIO3->PTOR = (1 << 13);
            usart0->driver->writeb(usart0, 'A');
        }
    }
}


