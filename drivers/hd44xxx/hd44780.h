#pragma once
#include <drivers/hd44xxx.h>


void hd44780_init(struct hd44xxx_device* display, struct hd44xxx_desc* desc);
int hd44780_ioctl(struct hd44xxx_device* display, int op, void* arg);

