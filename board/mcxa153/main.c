#include <stddef.h>
#include <kernel/board.h>
#include <kernel/majors.h>
#include <kernel/init.h>
#include <kernel/devtbl.h>

#include <drivers/usart.h>
#include <drivers/romdisk.h>

extern const uint8_t __rootfs_start[];
extern const uint8_t __rootfs_end[];

const device_node_t static_device_table[] = {
    {
        .major = USART_MAJOR,
        .preinit = 1,
        .desc = &(struct usart_desc) {
            .uart_num = 0,
            .baud = 10,
            .irq = LPUART0_IRQn,
            .type = MCXA_LPUART
        }
    },
    {
        .major = USART_MAJOR,
        .preinit = 0,
        .desc = &(struct usart_desc) {
            .uart_num = 1,
            .baud = 10,
            .irq = LPUART1_IRQn,
            .type = MCXA_LPUART
        }
    },
    {
        .major = ROMDISK_MAJOR,
        .preinit = 0,
        .desc = &(struct romdisk_desc) {
            .rom_base = __rootfs_start,
            .rom_end = __rootfs_end,
            .bsize = 512
        }
    }

};
const int static_device_table_size = 3;

[[gnu::aligned(8)]] uint8_t __heap_start[2816];
const int __heap_size = sizeof(__heap_start);

void main()
{
    kernel_pre_init();
    __enable_irq();

    dev_t usart0_devno;
    device_create(&usart0_devno, USART_MAJOR, static_device_table[0].desc);
   
    kernel_init();
}


