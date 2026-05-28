
__stack_size = 0x200;

SECTIONS
{
    . = 0x0;
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.header))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    }
    .data :
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
    }
    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
    }

    .stack :
    {
        . = ALIGN(8);
        __stack_bottom = .;
        . += __stack_size;
        . = ALIGN(8);
        __stack_top = .;
    }
}




