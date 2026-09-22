#pragma once
#include <board/board.h>
#include <kernel/devtbl.h>

extern uint8_t __heap_start[];
extern const int __heap_size;
extern const dt_node_t static_device_tree;

void system_init();
void system_blink();


