#include "st7920.h"
#include "port.h"

void st7920_init(struct hd44xxx_device* disp, struct hd44xxx_desc* desc)
{
    write_port(disp, 0, 0b00110000);
    tiny_delay(1000);
    write_port(disp, 0, 0b00110000);
    tiny_delay(1000);
    write_port(disp, 0, 0b00001111);
    tiny_delay(1000);
    write_port(disp, 0, 0b00000110);
    tiny_delay(1000);
    write_port(disp, 0, 0b00000001);
    tiny_delay(5000);
    
}

int st7920_ioctl(struct hd44xxx_device* disp, int cmd, void* op)
{
    return -1;
}

