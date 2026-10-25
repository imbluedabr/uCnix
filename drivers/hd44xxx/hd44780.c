#include "hd44780.h"
#include <kernel/time.h>
#include "port.h"

void hd44780_init(struct hd44xxx_device* display, const struct gpio_bus_desc* desc)
{
    //simple_delay(50);        // wait after power-up
	
    send_cmd(display, 0, 0b111000);
    simple_delay(5);

   	send_cmd(display, 0, 0b111000);
    simple_delay(1);

    send_cmd(display, 0, 0b111000);
    simple_delay(1);

    /*write_port(display, 0, 0b1111);   // 8-bit, 2-line, 5x8 font
    write_port(display, 0, 0x08);   // display off
    write_port(display, 0, 0x01);   // clear
    simple_delay(2);
    write_port(display, 0, 0x06);   // entry mode
    write_port(display, 0, 0x0C);   // display on
    */
    send_cmd(display, 0, 0b1111);
    simple_delay(1);
    send_cmd(display, 0, 0b110);
    simple_delay(1);
    send_cmd(display, 0, 0b1);
    simple_delay(5);
}

int hd44780_ioctl(struct hd44xxx_device* display, int op, void* arg)
{
    return 0;
}


