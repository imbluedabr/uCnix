
ENTRY(reset_handler)

STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0200;

/* Specify the memory areas */
MEMORY
{
  m_interrupts          (RX)    : ORIGIN = 0x00000000, LENGTH = 0x00000200
  m_flash               (RX)    : ORIGIN = 0x00000200, LENGTH = 0x00071E00
  m_sramx0              (RWX)   : ORIGIN = 0x04000000, LENGTH = 0x00008000
  m_sram0               (RWX)   : ORIGIN = 0x20000000, LENGTH = 0x00010000
  m_sram1               (RWX)   : ORIGIN = 0x20010000, LENGTH = 0x00010000
  m_sram2               (RWX)   : ORIGIN = 0x20020000, LENGTH = 0x00010000
  m_sram3               (RWX)   : ORIGIN = 0x20030000, LENGTH = 0x00010000
}

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

  .bss :
  {
    . = ALIGN(4);
    __bss_start = .;
    *(.bss)
    *(.bss*)
    . = ALIGN(4);
    __bss_end = .;
  } > m_sram0

  .data : AT(__DATA_ROM)
  {
    . = ALIGN(4);
    __data_start = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    __data_end = .;
  } > m_sram0
  
  .stack :
  {
    __stack_bottom = .;
    . = ALIGN(8);
    . += STACK_SIZE;
    __stack_top = .;
  } > m_sram0

  
  .userspace1 :
  {
    __userspace_start = .;
    . += 64K;
  } > m_sram1

  .userspace2 :
  {
    . += 64K;
  } > m_sram2

  .userspace3 :
  {
    . += 64K;
    __userspace_end = .;
  } > m_sram3
}
