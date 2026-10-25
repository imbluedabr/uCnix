#pragma once

#include <kernel/device.h>
#include <fs/vfs.h>
#include <stddef.h>
#include <stdint.h>

typedef enum : uint8_t {
    HD_ST_9720,
    HD_44780_1602,
    HD_44780_4002
} hd44xxx_type_e;

struct hd44xxx_device {
    struct device base;
	uint32_t pin_mask;
	uint8_t impl;
    uint8_t screen_end;
    uint8_t cursor_x : 5;
    uint8_t cursor_y : 3;
    //flags
    uint8_t newl_pending : 1;
    uint8_t graphics_mode : 1;
};

struct hd44xxx_impl {
    void (*init)(struct hd44xxx_device* display, const struct gpio_bus_desc* desc);
    int (*ioctl)(struct hd44xxx_device* display, int op, void* arg);
};

//register the tty driver
void hd44xxx_init();

//create a tty instance, args is a tty_hardware_interface pointer to the tty
struct device* hd44xxx_probe(struct bus_device* parent, const void* descriptor);
int hd44xxx_ll_write(struct device* dev, const void* buff, size_t count);
int hd44xxx_ioctl(struct file* f, int cmd, void* arg);

