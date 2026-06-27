#pragma once
#include <stdint.h>

#define TINY_EXEC_MAGIC "TEX"
#define TINY_EXEC_OSABI_MAJOR(ABI) (ABI >> 4)
#define TINY_EXEC_OSABI_MINOR(ABI) (ABI & 0xF)

typedef enum : uint8_t {
    ARMV8_M_MAIN = 1,
    ARMV8_M_BASE = 2
} tiny_exec_arch_e;

typedef struct {
    char magic[4];
    tiny_exec_arch_e arch;
    uint8_t os_abi;
    uint16_t padding0;
    uint32_t program_break;
    uint32_t stack_size;
    uint32_t entry_point;
    uint32_t rel_start;
    uint32_t rel_end;
} tiny_exec_hdr_t;


#define R_ARM_RELATIVE 23
typedef struct {
    uint32_t r_offset;  // where to patch (offset in image)
    uint32_t r_info;    // type + symbol index
} elf_rel_t;

