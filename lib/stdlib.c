#include <lib/stdlib.h>
#include <stdbool.h>

uint32_t strlen(const char* str)
{
    int size = 0;
    while (*str++ != '\0') {
        size++;
    }
    return size;
}

uint32_t strnlen(const char* str, uint32_t max_size)
{
    uint32_t size = 0;
    while (*str++ != '\0') {
        if (size > max_size) {
            break;
        }
        size++;
    }
    return size;
}


int strncmp( const char * s1, const char * s2, uint32_t n )
{
    while ( n && *s1 && ( *s1 == *s2 ) )
    {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 )
    {
        return 0;
    }
    else
    {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

int strncpy(char* dest, char* src, uint32_t size)
{
    int i = 0;
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
        i++;
    }
    return i;
}

int strlcpy(char* dest, const char* src, uint32_t size)
{
    int i = 0;
    while((*src != '\0') && (size-- > 0)) {
        *(dest++) = *(src++);
        i++;
    }
    *dest = '\0';
    return i;
}


[[gnu::used]] void memset(void* dest, uint8_t value, uint32_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = value;
    }
}


[[gnu::used]] void* memcpy(void* dest, const void* source, size_t size)
{
    while(size-- > 0) {
        ((uint8_t*) dest)[size] = ((uint8_t*) source)[size];
    }
    return dest;
}

//props to geeksforgeeks
// A utility function to reverse a string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}
// Implementation of citoa()
char* itoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}


