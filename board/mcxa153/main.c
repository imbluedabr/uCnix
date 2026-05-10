#include <stddef.h>
#include <kernel/board.h>
#include <kernel/majors.h>
#include <kernel/init.h>
#include <kernel/devtbl.h>

#include <drivers/usart.h>
#include <drivers/hd44xxx.h>
#include <drivers/romdisk.h>


/*
[[gnu::aligned(8)]] uint8_t test2_ustack[256];
[[gnu::aligned(8)]] uint8_t test2_kstack[256];
uint32_t test2_time;
[[gnu::naked]] int __SVC(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    __asm volatile(
        "svc #0\n"
        "bx lr"
    );
} */

extern const uint8_t __rootfs_start[];
extern const uint8_t __rootfs_end[];

const device_node_t static_device_table[] = {
    {
        .major = USART_MAJOR,
        .preinit = 1,
        .desc = &(struct usart_desc) {
            .base = LPUART0,
            .baud = 5,
            .irq = LPUART0_IRQn,
            .type = MCXA_LPUART
        }
    },
/*    {
        .major = HD44XXX_MAJOR,
        .preinit = 1,
        .desc = &(struct hd44xxx_desc) {
            .port = NULL,
            .type = HD_44780_4002
        }
    },*/
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

[[gnu::aligned(8)]] uint8_t __heap_start[2048];

void main()
{
    kernel_pre_init();
    __enable_irq();

    dev_t usart0_devno;
    device_create(&usart0_devno, USART_MAJOR, static_device_table[0].desc);
    
    kernel_init();
}


