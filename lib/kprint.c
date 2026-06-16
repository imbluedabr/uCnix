#include <lib/kprint.h>
#include <stdint.h>
#include <stdarg.h>
#include <lib/hex.h>
#include <lib/stdlib.h>
#include <kernel/device.h>
#include <kernel/lock.h>
#include <kernel/time.h>


struct device* boot_console;

mutex_t console_lock;

void kputc(char c)
{
    while(boot_console->driver->writeb(boot_console, c) == -1) {
        //boot_console->driver->update(boot_console);
    };
}

void kputs(const char *str)
{
    while (*str != '\0')
    {
        kputc(*str++);
    }
}

void kvprintf(const char *fmt, va_list params)
{
    static char temp_buff[32];

    while (*fmt != '\0') {
        if (*fmt != '%') kputc(*fmt);
        else {
            fmt++;

            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                kputc(ch);
            } else if (*fmt == 's') {
                char *s = va_arg(params, char *);
                kputs(s);
            } else if (*fmt == 'x') {
                int num = va_arg(params, int);
                kputs(itoa(num, temp_buff, 16));
            } else if (*fmt == 'd') {
                int num = va_arg(params, int);
                kputs(itoa(num, temp_buff, 10));
            }
        }
        fmt++;
    }
}

void kprintf(const char *fmt, ...) 
{
    va_list params;
    va_start(params, fmt);
    mutex_lock(&console_lock);
    kvprintf(fmt, params);
    mutex_unlock(&console_lock);
    va_end(params);
}

static const char error_names[][13] = {
    "\e[1;30mDEBUG",
    "\e[1;34mINFO",
    "\e[1;33mWARN",
    "\e[1;31mERROR"
};

void klog(log_e level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    mutex_lock(&console_lock);

    kprintf("[%d] <%s\e[1;39m> ", get_kernel_ticks(), error_names[level]);
    

    kvprintf(fmt, args);

    mutex_unlock(&console_lock);
    va_end(args);
}



