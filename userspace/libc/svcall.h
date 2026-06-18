#pragma once

#define SVCALL(SVCNO) __asm volatile( \
            "svc #" #SVCNO "\nbx lr" \
            )




