__stack_size = 0x200;

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x0;
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.header))
        *(.text)
        *(.text*)
        . = ALIGN(4);
    } :text
    
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } :text
    
    .rel :
    {
        . = ALIGN(4);
        __rel_start = .;
        KEEP(*(.rel))
        KEEP(*(.rel*))
        . = ALIGN(4);
        __rel_end = .;
    } :text

    .got :
    {
        . = ALIGN(4);
        __got_start = .;
        KEEP(*(.got))
        KEEP(*(.got*))
        . = ALIGN(4);
        __got_end = .;
    } :data


    .data :
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
    } :data

    .bss :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(4);
        __bss_end = .;
    } :data

    .stack :
    {
        . = ALIGN(8);
        __stack_bottom = .;
        . += __stack_size;
        . = ALIGN(8);
        __stack_top = .;
    } :data

    /DISCARD/ :
    {
        *(.interp)
        *(.dynsym)
        *(.dynstr)
        *(.hash)
        *(.dynamic)
        *(.plt)
        *(.plt*)
        *(.comment)
        *(.ARM.attributes)
        *(.note*)
        *(.eh_frame*)
        *(.symtab)
        *(.strtab)
        *(.shstrtab)
    }
}

