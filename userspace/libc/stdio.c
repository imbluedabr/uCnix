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

void vsnprintf(char* buff, int size, const char* fmt, va_list params) {
    int i = 0;
    size--;
    while (*fmt && i < size) {
        if (*fmt != '%') buff[i++] = *fmt;
        else {
            fmt++;
            if (*fmt == 'c') {
                char ch = va_arg(params, int);
                buff[i++] = ch;
            } else if (*fmt == 's') {
                char* s = va_arg(params, char*);
                i += strncpy(buff + i, s, size - i);
            } else if (*fmt == 'x') {
                int num = va_arg(params, int);
                i += itona(num, buff + i, size - i, 16);
            } else if (*fmt == 'o') {
                int num = va_arg(params, int);
                i += itona(num, buff + i, size - i, 8);
            } else if (*fmt == 'd') {
                int num = va_arg(params, int);
                i += itona(num, buff + i, size - i, 10);
            }
        }
        fmt++;
    }
    //terminate with null character
    buff[i] = '\0';
}

void snprintf(char* buff, int size, const char* fmt, ...) {
    va_list params;
    va_start(params, fmt);
    vsnprintf(buff, size, fmt, params);
    va_end(params);
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

