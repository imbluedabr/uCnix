#pragma once
#include <stdarg.h>

void putc(char c);
void puts(const char* c);
void vprintf(const char *fmt, va_list params);
void printf(const char* fmt, ...);

