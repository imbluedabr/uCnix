#pragma once
#include <kernel/settings.h>
#include <board/board.h>
#include <drivers/hd44xxx.h>

static inline void tiny_delay(int loops)
{
    for (volatile int i = 0; i < loops; i++) {
        __NOP();
    }
}

void send_cmd(struct hd44xxx_device* display, uint8_t rs, uint8_t val);


