#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <unistd.h>

void putc(char c)
{
    write(STDOUT_FILENO, &c, 1);
}

void puts(const char* c)
{
    write(STDOUT_FILENO, c, strlen(c));
}

void vprintf(const char *fmt, va_list params)
{
    static char temp_buff[32];

    while (*fmt != '\0') {
        if (*fmt != '%') putc(*fmt);
        else {
            fmt++;

            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                putc(ch);
            } else if (*fmt == 's') {
                char *s = va_arg(params, char *);
                puts(s);
            } else if (*fmt == 'x') {
                int num = va_arg(params, int);
                puts(itoa(num, temp_buff, 16));
            } else if (*fmt == 'o') {
                int num = va_arg(params, int);
                puts(itoa(num, temp_buff, 8));
            } else if (*fmt == 'd') {
                int num = va_arg(params, int);
                puts(itoa(num, temp_buff, 10));
            }
        }
        fmt++;
    }
}

void printf(const char *fmt, ...) 
{
    va_list params;
    va_start(params, fmt);
    vprintf(fmt, params);
    va_end(params);
}

