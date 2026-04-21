#include <kernel/device.h>
#include <stdarg.h>

extern struct device* boot_console;


void kputc(char c);
void kputs(const char* str);
void kprintf(const char* fmt, ...);


