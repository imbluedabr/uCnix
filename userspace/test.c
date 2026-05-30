#include <stdint.h>

#define SVCALL(SVCNO) __asm volatile( \
            "svc #" #SVCNO "\nbx lr" \
            )

[[gnu::naked]] int write(int fd, const void* buffer, int count) {
    SVCALL(1);
}

[[gnu::naked]] void exit(int exit_code) {
    SVCALL(43);
}


int main() {

    //__asm volatile ("bkpt\n");

    write(0, "Hello world!\n", 13);
    
    exit(0);
    while(1);
}

