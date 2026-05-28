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
    uint32_t padding1;
} tiny_exec_hdr_t;

