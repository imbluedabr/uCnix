#include <stdint.h>

[[gnu::naked]] int write(int fd, const void* buffer, int count) {
    __asm volatile(
        "svc #0\n"
            "bx lr"
    );
}


int main() {
    
    write(0, "Hello world!\n", 13);

    while(1);
}

