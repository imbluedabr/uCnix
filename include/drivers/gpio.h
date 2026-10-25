#pragma once
#include <kernel/device.h>
#include "fs/vfs.h"


struct gpio_bus_device {
	struct bus_device base;
	volatile void* port;
	volatile void* gpio;
};

void gpio_init();
struct device* gpio_probe(struct bus_device* parent, const void* descriptor); //create gpio bus
struct device* gpio_bus_probe(struct bus_device* bus, const void* desc); //create device on gpio bus
int gpio_range_claim(struct bus_device* bus, int mask, int mode); //claim all the pins in the mask
int gpio_range_write(struct bus_device* bus, int mask, int value); //write each pin in the mask with the bit in the value
int gpio_range_read(struct bus_device* bus, int mask); //read all the pins in the mask
int gpio_range_unclaim(struct bus_device* bus, int mask); //unclaim the pins in the mask
int gpio_ioctl(struct file* f, int cmd, void* arg);

