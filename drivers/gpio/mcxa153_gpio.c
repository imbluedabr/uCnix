#include "board/mcxa153/MCXA153_COMMON.h"
#include "kernel/device.h"
#include <uapi/sys/errno.h>
#include <drivers/gpio.h>
#include <kernel/majors.h>
#include <kernel/board.h>
#include <lib/kmalloc.h>
#include <stdint.h>
#include <uapi/sys/gpio.h>

struct bus_driver gpio_driver = {
	.base.probe = &gpio_probe,
	.base.name = "gpio",
	.base.bus_accept = BUS_MMIO,
	.base.bus_type = BUS_GPIO
};

static const struct dev_ops gpio_ops = {
	.ioctl = gpio_ioctl
};

static const struct bus_ops gpio_bus_ops = {
	.probe = gpio_bus_probe,
	.gpio_range_claim = gpio_range_claim,
	.gpio_range_write = gpio_range_write,
	.gpio_range_read = gpio_range_read,
	.gpio_range_unclaim = gpio_range_unclaim
};

void gpio_init()
{
	register_driver(GPIO_MAJOR, &gpio_driver.base);
}

struct device* gpio_probe(struct bus_device* parent, const void* descriptor)
{
    struct gpio_bus_device* dev = kzalloc(sizeof(struct gpio_bus_device));
    if (!dev) return NULL;
    const struct mmio_bus_desc* desc = descriptor;
    
	dev->base.base.ops = (struct dev_ops*) &gpio_ops;
	dev->base.bus_ops = (struct bus_ops*) &gpio_bus_ops;
   	dev->gpio = desc->base;
	switch((uint32_t) desc->base) {
		case GPIO0_BASE:
			MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO0(1);
			MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT0(1);
			MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO0(1);
			MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT0(1);
			dev->port = PORT0;
			break;
		case GPIO1_BASE:
			MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1(1);
			MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT1(1);
			MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1(1);
			MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT1(1);
			dev->port = PORT1;
			break;
		case GPIO2_BASE:
			MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO2(1);
			MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT2(1);
			MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO2(1);
			MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT2(1);
			dev->port = PORT2;
			break;
		case GPIO3_BASE:
			MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3(1);
			MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC1_PORT3(1);
			MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3(1);
			MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST1_PORT3(1);
			dev->port = PORT3;
			break;
	}

    return &dev->base.base;
}

struct device* gpio_bus_probe(struct bus_device* bus, const void* desc) //create device on gpio bus
{
	struct gpio_bus_device* dev = (struct gpio_bus_device*) bus;
	const struct gpio_bus_desc* gpio_desc = desc;
	
	//simply match the descriptor against registered drivers
	for (int i = 0; i < gpio_driver.registered_drivers_count; i++) {
		if (gpio_driver.registered_drivers[i] == gpio_desc->major) {
			return device_probe(gpio_desc->major, bus, desc);
		}
	}

	return NULL;
}

int gpio_range_claim(struct bus_device* bus, int mask, int mode) //claim all the pins in the mask
{
	struct gpio_bus_device* dev = (struct gpio_bus_device*) bus;
	volatile GPIO_Type* gpio = dev->gpio;
	volatile PORT_Type* port = dev->port;
	
	for (int i = 0; i < 32; i++) {
		if (mask & (1 << i)) {
			if (mode == GPIO_PINMODE_INPUT) {
				port->PCR[i] = PORT_PCR_IBE(1);
			} else {
				//port->PCR[i] = 
			}
			uint32_t reg = gpio->PDDR & ~(1<<i);
			if (mode == GPIO_PINMODE_OUTPUT) {
				reg |= (1<<i);
			}
			gpio->PDDR = reg;

		}
	}
	gpio->PCOR |= mask;
	return 0;
}

int gpio_range_write(struct bus_device* bus, int mask, int value) //write each pin in the mask with the bit in the value
{
	struct gpio_bus_device* dev = (struct gpio_bus_device*) bus;
	volatile GPIO_Type* gpio = dev->gpio;
	
	int val = gpio->PDOR;
	val &= ~(mask);
	val |= value;
	gpio->PDOR = val;

	return 0;
}

int gpio_range_read(struct bus_device* bus, int mask) //read all the pins in the mask
{
	struct gpio_bus_device* dev = (struct gpio_bus_device*) bus;
	volatile GPIO_Type* gpio = dev->gpio;
	return gpio->PDIR & mask;
}

int gpio_range_unclaim(struct bus_device* bus, int mask) //unclaim the pins in the mask
{
	return 0;
}

int gpio_ioctl(struct file* f, int cmd, void* arg)
{
	struct gpio_bus_device* dev = (struct gpio_bus_device*) f->i->devfs.dev;
	const struct gpio_cmd* command = arg;
	
	switch(cmd) {

		case IOCTL_GPIO_CLAIM:
			return gpio_range_claim(&dev->base, command->mask, command->value);
		case IOCTL_GPIO_WRITE:
			return gpio_range_write(&dev->base, command->mask, command->value);
		case IOCTL_GPIO_READ:
			return gpio_range_read(&dev->base, command->mask);
		case IOCTL_GPIO_UNCLAIM:
			return gpio_range_unclaim(&dev->base, command->mask);
		default:
			return -ENOTTY;
	}
}




