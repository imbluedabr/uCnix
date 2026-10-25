#pragma once
#include "kernel/device.h"
#include <drivers/hd44xxx.h>

void st7920_init(struct hd44xxx_device* disp, const struct gpio_bus_desc* desc);
int st7920_ioctl(struct hd44xxx_device* disp, int cmd, void* op);



