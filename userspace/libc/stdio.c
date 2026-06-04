#include "stdio.h"
#include "string.h"
#include <unistd.h>

void putc(char c)
{
    write(STDOUT_FILENO, &c, 1);
}

void puts(const char* c)
{
    write(STDOUT_FILENO, c, strlen(c));
}

void printf(const char* fmt, ...)
{

}

