
ENTRY(reset_handler)

STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0200;

/* Specify the memory areas */
MEMORY
{
  m_interrupts          (RX)  : ORIGIN = 0x00000000, LENGTH = 0x00000200
  m_flash                (RX)  : ORIGIN = 0x00000200, LENGTH = 0x0001FE00
  m_srama0              (RWX)  : ORIGIN = 0x20000000, LENGTH = 0x00002000
  m_srama1x0            (RWX)  : ORIGIN = 0x20002000, LENGTH = 0x00006000
  m_sramx0              (RWX)  : ORIGIN = 0x04000000, LENGTH = 0x00002000
}
__userspace_size = 0x6000;

SECTIONS
{
  .interrupts :
  {
    . = ALIGN(4);
    KEEP(*(.boot_vtable)) /* early boot vector table until the ram vector table is initialized */
    . = ALIGN(4);
  } > m_interrupts

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > m_flash

  .rootfs :
  {
    . = ALIGN(4);
    __rootfs_start = .;
    KEEP(rootfs.o(.data))
    __rootfs_end = .;
    . = ALIGN(4);
  } > m_flash
  
  __DATA_ROM = .;
  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __data_start = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    __data_end = .;
  } > m_srama0
  __DATA_ROM_END = .;

  .bss :
  {
    . = ALIGN(4);
    __bss_start = .;
    *(.bss)
    *(.bss*)
    . = ALIGN(4);
    __bss_end = .;
  } > m_srama0
  
  .stack :
  {
    __stack_bottom = .;
    . = ALIGN(8);
    . += STACK_SIZE;
    __stack_top = .;
  } > m_srama0

  .userspace :
  {
    __userspace_start = .;
    . += __userspace_size;
    __userspace_end = .;
  } > m_srama1x0
}
