#include <lib/kprint.h>
#include <kernel/interrupt.h>

//stack protector lol
uint32_t __stack_chk_guard = 0xDEADBEEF;


//just praying that this atleast catches the frigin error
void __stack_chk_fail(void) {

    kerr("stack check fail!\n");
    disable_interrupts();
    while(1);
}





