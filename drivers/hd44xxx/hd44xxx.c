#include <drivers/hd44xxx.h>
#include <kernel/majors.h>
#include <lib/kprint.h>
#include <lib/kmalloc.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "kernel/device.h"
#include "st7920.h"
#include "hd44780.h"
#include "uapi/sys/gpio.h"

static const uint8_t DDRAM_addr[][4] = {
    [HD_ST_9720] = {
        0x00,
        0x10,
        0x8,
        0x18,
    },
    [HD_44780_1602] = {
        0x00,
        0x40
    },
    [HD_44780_4002] = {
        0x00,
        0x40,
        0x14,
        0x54
    }
};

static const uint8_t display_format[][2] = {
    [HD_ST_9720] = {
        0x10,
        0x04
    },
    [HD_44780_1602] = {
        0x10,
        0x02
    },
    [HD_44780_4002] = {
        0x14,
        0x04
    }
};

static const struct hd44xxx_impl disp_impl[] = {
    [HD_ST_9720] = {
        &st7920_init,
        &st7920_ioctl
    },
    [HD_44780_1602] = {
        &hd44780_init,
        &hd44780_ioctl
    },
    [HD_44780_4002] = {
        &hd44780_init,
        &hd44780_ioctl
    }
};

struct device_driver hd44xxx_driver = {
	.probe = hd44xxx_probe,
    .name = "lcd",
	.bus_accept = BUS_GPIO
};


void hd44xxx_init() {
    register_driver(HD44XXX_MAJOR, &hd44xxx_driver);
}

#define RS_SHIFT 8
#define RW_MASK (1 << 10)
#define EN_MASK (1 << 9)

void send_cmd(struct hd44xxx_device* display, uint8_t rs, uint8_t val) {
	struct bus_device* bus = display->base.parent;
	bus->bus_ops->gpio_range_write(bus, 0x7FF, (rs << RS_SHIFT) | val);
    //tiny_delay(10);
	bus->bus_ops->gpio_range_write(bus, EN_MASK, EN_MASK);
    //tiny_delay(20);
	bus->bus_ops->gpio_range_write(bus, EN_MASK, 0);
    //tiny_delay(10);
	bus->bus_ops->gpio_range_write(bus, 0x7FF, 0);
}

static int recieve_cmd(struct hd44xxx_device* display, uint8_t rs) {
    uint8_t recieved;
	struct bus_device* bus = display->base.parent;
	bus->bus_ops->gpio_range_claim(bus, 0xFF, GPIO_PINMODE_INPUT);
	bus->bus_ops->gpio_range_write(bus, 0x700, (rs << RS_SHIFT) | RW_MASK);
    //tiny_delay(10);
	bus->bus_ops->gpio_range_write(bus, EN_MASK, EN_MASK);
    //tiny_delay(20);
	recieved = bus->bus_ops->gpio_range_read(bus, 0xFF);
    //tiny_delay(20);
	bus->bus_ops->gpio_range_write(bus, 0x700, 0);
	//tiny_delay(10);
	bus->bus_ops->gpio_range_claim(bus, 0xFF, GPIO_PINMODE_OUTPUT);
    return recieved;
}

static void cmd_display_clear(struct hd44xxx_device* disp)
{
    send_cmd(disp, 0, 1);
}

static void cmd_set_ddram(struct hd44xxx_device* disp, uint8_t val)
{
    send_cmd(disp, 0, 0x80 | (val & 0x7F));
}


struct device* hd44xxx_probe(struct bus_device* parent, const void* descriptor)
{
    struct hd44xxx_device* display = kzalloc(sizeof(struct hd44xxx_device));
    if (display == NULL) {
        return NULL;
    }
    struct gpio_bus_desc* desc = (struct gpio_bus_desc*) descriptor;
    display->impl = desc->vendor_id;

	parent->bus_ops->gpio_range_claim(parent, 0x7FF, GPIO_PINMODE_OUTPUT);
	display->base.parent = parent;
	
    disp_impl[desc->vendor_id].init(display, desc);

    return &display->base;
}

int hd44xxx_ioctl(struct file* f, int cmd, void* arg)
{
    struct hd44xxx_device* disp = (struct hd44xxx_device*) f->i->devfs.dev;

    return disp_impl[disp->impl].ioctl(disp, cmd, arg);
}

static inline void writeb(struct hd44xxx_device* display, char c)
{
	while (recieve_cmd(display, 0) & 0x70);
	send_cmd(display, 1, c);
	display->cursor_x++;
	if (display->cursor_x == display_format[display->impl][0]) {
		display->cursor_x = 0;
		display->cursor_y++;
		cmd_set_ddram(display, DDRAM_addr[display->impl][display->cursor_y]);
	}
	if (display->cursor_y == display_format[display->impl][1]) {
		display->cursor_y = 0;
		cmd_display_clear(display);
	}
}

int hd44xxx_ll_write(struct device* dev, const void* buff, size_t count)
{
    struct hd44xxx_device* disp = (struct hd44xxx_device*) dev;
	
	for (size_t i = 0; i < count; i++) {
		writeb(disp, ((char*)buff)[i]);
	}

    return count;
}


