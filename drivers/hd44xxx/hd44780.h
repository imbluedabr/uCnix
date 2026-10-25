#pragma once
#include <kernel/device.h>
#include <drivers/hd44xxx.h>


void hd44780_init(struct hd44xxx_device* display, const struct gpio_bus_desc* desc);
int hd44780_ioctl(struct hd44xxx_device* display, int op, void* arg);

