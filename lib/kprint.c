#include <lib/kprint.h>
#include <stdint.h>
#include <stdarg.h>
#include <lib/hex.h>
#include <kernel/device.h>


struct device* boot_console;

void kputc(char c)
{
    while(boot_console->driver->writeb(boot_console, c) == -1) {
        boot_console->driver->update(boot_console);
    };
}

void kputs(const char *str)
{
    while (*str != '\0')
    {
        kputc(*str++);
    }
}

void kprintf(const char *fmt, ...) 
{
    va_list params;
    va_start(params, fmt);

    while (*fmt != '\0') {
        if (*fmt != '%') kputc(*fmt);
        else {
            fmt++;

            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                kputc(ch);
            }
            else if (*fmt == 's') {
                char *s = va_arg(params, char *);
                kputs(s);
            }
            else if (*fmt == 'x') {
                int num = va_arg(params, int);
                const char *hex = u32_to_hex(num);
                kputs(hex);
            }
        }
        fmt++;
    }
}
