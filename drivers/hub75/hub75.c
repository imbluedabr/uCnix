#include "uapi/sys/types.h"
#include <kernel/device.h>
#include <kernel/majors.h>
#include <kernel/interrupt.h>
#include <drivers/hub75.h>
#include <fs/vfs.h>
#include <lib/kmalloc.h>
#include <stdint.h>
#include <uapi/sys/gpio.h>
#include <uapi/sys/errno.h>

struct device_driver  hub75_driver = {
	.probe = &hub75_probe,
	.name = "hub75-",
	.bus_accept = BUS_GPIO
};

static const struct dev_ops hub75_ops = {
	.write = hub75_write,
	.ioctl = hub75_ioctl
};

void hub75_init()
{
	register_driver(HUB75_MAJOR, &hub75_driver);
}

struct device* hub75_probe(struct bus_device* parent, const void* descriptor)
{
	struct hub75_device* dev = (struct hub75_device*) kzalloc(sizeof(struct hub75_device));
	dev->base.ops = (struct dev_ops*) &hub75_ops;
	dev->x_size = 128;
	dev->y_size = 64;
	dev->scan_size = 32;

	parent->bus_ops->gpio_range_claim(parent, 0x3FFF, GPIO_PINMODE_OUTPUT);
	
	return &dev->base;
}

#define PIN_OE (1 << 7)
#define PIN_CLK (1 << 6)
#define PIN_LAT (1 << 5)
#define ADDR_MASK 0x1F
#define RGB1(X) ((X & 0x7) << 8)
#define RGB2(X) ((X & 0x7) << 11)

ssize_t hub75_write(struct file* f, const void* buff, size_t count)
{
	struct hub75_device* dev = (struct hub75_device*) f->i->devfs.dev;
	struct bus_device* bus = dev->base.parent;
	const uint8_t* framebuffer = (const uint8_t*) buff; //uint8_t framebuffer[y][x/8] monochrome fb
	
	int irq = disable_interrupts();
	
	for (int k = 0; k < dev->scan_size; k++) {

		bus->bus_ops->gpio_range_write(bus, PIN_OE | ADDR_MASK, PIN_OE | k);
		
		for (int i = 0; i < dev->x_size; i++) { //only writing the upper half of the screen since i have not wired RGB2 yet
			uint16_t index1 = k*(dev->x_size/8) + (i/8); //upper half of the screen
			uint16_t index2 = (k+dev->scan_size)*(dev->x_size/8) + (i/8); //lower half of the screen
			uint8_t bits1 = framebuffer[index1];
			uint8_t bits2 = framebuffer[index2];
			bus->bus_ops->gpio_range_write(bus, PIN_CLK | RGB1(0x7), PIN_CLK | RGB1(!!(bits1 & (1 << (i & 0x7)))));
			bus->bus_ops->gpio_range_write(bus, PIN_CLK, 0);
		}

		bus->bus_ops->gpio_range_write(bus, PIN_LAT, PIN_LAT);
		bus->bus_ops->gpio_range_write(bus, PIN_LAT, 0);

		bus->bus_ops->gpio_range_write(bus, PIN_OE, 0);

		for (volatile int i = 0; i < 1000; i++);
	}
	
	enable_interrupts(irq);


	return dev->y_size;
}

int hub75_ioctl(struct file* f, int cmd, void* arg)
{
	return -ENOSYS;
}

