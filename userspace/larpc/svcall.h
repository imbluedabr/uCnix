#pragma once

#define SVCALL(n) __asm volatile( \
    "svc %[imm]\n" \
    "bx lr\n" \
    : : [imm] "I" (n) \
)

