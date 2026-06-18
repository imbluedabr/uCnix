#pragma once
#include <board/board.h>

extern uint8_t __heap_start[];
extern const int __heap_size;

void system_init();
void system_blink();


