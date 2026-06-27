#pragma once
#include <stdarg.h>

void putc(char c);
void puts(const char* c);
void vsnprintf(char* buff, int size, const char* fmt, va_list params);
void snprintf(char* buff, int size, const char* fmt, ...);
void vprintf(const char *fmt, va_list params);
void printf(const char* fmt, ...);

