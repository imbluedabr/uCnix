#include <stddef.h>
#include <kernel/board.h>
#include <kernel/majors.h>
#include <kernel/init.h>
#include <kernel/devtbl.h>

#include <drivers/usart.h>
#include <drivers/romdisk.h>

extern const uint8_t __rootfs_start[];
extern const uint8_t __rootfs_end[];

static const struct mmio_bus_desc usart0_desc = {
	.base = (uint8_t*) LPUART0,
	.vendor_id = USART_MCXA,
	.major = USART_MAJOR,
	.irq = LPUART0_IRQn
};

const dt_node_t static_device_tree = {
	.child = &(dt_node_t) {
		.preinit = 1,
		.desc = &usart0_desc,
	.next = &(dt_node_t) {
		.preinit = 0,
		.desc = &(struct mmio_bus_desc) {
			.base = __rootfs_start,
			.size = (size_t) __rootfs_end,
			.major = ROMDISK_MAJOR
		}
	}
	}
};

[[gnu::aligned(8)]] uint8_t __heap_start[2816];
const int __heap_size = sizeof(__heap_start);

void main()
{
    kernel_pre_init();
    __enable_irq();
	
	//initialize the boot console
    device_probe(USART_MAJOR, NULL, &usart0_desc);
   
    kernel_init();
}


