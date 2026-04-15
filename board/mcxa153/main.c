#include <stddef.h>
#include <board/mcxa153/board.h>
#include <kernel/interrupt.h>
#include <kernel/alloc.h>
#include <kernel/time.h>
#include <kernel/device.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <uapi/majors.h>
#include <drivers/usart.h>
#include <drivers/hd44xxx.h>


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

[[gnu::aligned(8)]] char heap[2048];
struct device* usart0;

[[gnu::aligned(8)]] uint8_t test1_stack[128];
uint32_t test1_time;
void test1_process()
{
    __BKPT();
    while(1) {
        if ((get_kernel_ticks() - test1_time) > 1000) {
            __BKPT();
            test1_time = get_kernel_ticks();
            GPIO3->PTOR = 1 << 13;
            //usart0->driver->writeb(usart0, 'A');
        }
        __WFI();
    };
}

[[gnu::aligned(8)]] uint8_t test2_stack[128];
uint32_t test2_time;
void test2_process()
{
    __BKPT();
    while(1) {
        if ((get_kernel_ticks() - test2_time) > 1000) {
            test2_time = get_kernel_ticks();
            GPIO3->PTOR = 1 << 13;
        }
        __WFI();
    };
}


void main()
{
    system_init(); //initialize cache, flash and other basic board specific stuff
    interrupt_init(); //load the ram vector table
    init_heap(&heap, 2048); //initialize the heap
    device_init();
    proc_init();
    syscall_init();
    usart_init();
    hd44xxx_init();
    gpio_output_init();
    
    dev_t usart0_devno;
    usart0 = device_create(&usart0_devno, USART_MAJOR, &(struct usart_desc) {
            .base = LPUART0,
            .baud = 5, //baud rate is 9600
            .irq = LPUART0_IRQn,
            .type = MCXA_LPUART
            });
    dev_t display_devno;
    struct device* display = device_create(&display_devno, HD44XXX_MAJOR, &(struct hd44xxx_desc) {
            .port = NULL,
            .type = HD_44780_4002
            });
    
    //proc_create(test1_stack, 128, &test1_process);
    //proc_create(test2_stack, 128, &test2_process);
    
    //proc_start_scheduling();
    //__BKPT();
    SysTick_Config(48000);
    __enable_irq();
    uint32_t last_update = 0;
    while(1) {
        if ((get_kernel_ticks() - last_update) > 1000) {
            last_update = get_kernel_ticks();
            GPIO3->PTOR = (1 << 13);
            display->driver->writeb(display, 'A');
            //usart0->driver->writeb(usart0, 'A');
        }
        display->driver->update(display);
    }
}


