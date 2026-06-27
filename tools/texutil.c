#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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
    uint32_t r_offset;  //location in binary
    uint32_t r_info;    //type
} elf_rel_t;

tiny_exec_hdr_t program_hdr;

int main(int argc, char** argv)
{
    int exec = open(argv[1], O_RDONLY);

    read(exec, &program_hdr, sizeof(tiny_exec_hdr_t));
    lseek(exec, 0, SEEK_SET);
    
    printf("tiny_exec_hdr_t:\n\tmagic: %s\n\tarch: %d\n\tos_abi: %d\n\tprogram_break: %d\n\tstack_size: %d\n\tentry_point: %d\n", program_hdr.magic, program_hdr.arch, program_hdr.os_abi, program_hdr.program_break, program_hdr.stack_size, program_hdr.entry_point);

    printf("rel:\n\trel_start: %d\n\trel_end: %d\n", program_hdr.rel_start, program_hdr.rel_end);
    
    uint8_t* program_base = malloc(program_hdr.program_break);
    read(exec, program_base, program_hdr.program_break);

    int rel_size = (program_hdr.rel_end - program_hdr.rel_start)/sizeof(elf_rel_t);
    elf_rel_t* rel = (elf_rel_t*)(program_base + program_hdr.rel_start);
    printf("enumerating rel:\n\trel_size: %d\n", rel_size);
    for (int i = 0; i < rel_size; i++) {
        uint32_t* val = (uint32_t*) (program_base + rel[i].r_offset);
        printf("\toffset=%d, info=%d, val=%d\n", rel[i].r_offset, rel[i].r_info, *val);
    }
    

}





