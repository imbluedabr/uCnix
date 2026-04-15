#include "hd44780.h"
#include "port.h"

void hd44780_init(struct hd44xxx_device* display, struct hd44xxx_desc* desc)
{
    write_port(display, 0, 0b111000);
    tiny_delay(5000);
    write_port(display, 0, 0b111000);
    tiny_delay(5000);
    write_port(display, 0, 0b1111);
    tiny_delay(5000);
    write_port(display, 0, 0b110);
    tiny_delay(5000);
    write_port(display, 0, 0b1);
    tiny_delay(5000);

}

int hd44780_ioctl(struct hd44xxx_device* display, int op, void* arg)
{

}


