#include "st7920.h"
#include "port.h"
#include <kernel/device.h>

void st7920_init(struct hd44xxx_device* disp, const struct gpio_bus_desc* desc)
{
    send_cmd(disp, 0, 0b00110000);
    tiny_delay(1000);
    send_cmd(disp, 0, 0b00110000);
    tiny_delay(1000);
    send_cmd(disp, 0, 0b00001111);
    tiny_delay(1000);
    send_cmd(disp, 0, 0b00000110);
    tiny_delay(1000);
    send_cmd(disp, 0, 0b00000001);
    tiny_delay(5000);
}

int st7920_ioctl(struct hd44xxx_device* disp, int cmd, void* op)
{
    return -1;
}

