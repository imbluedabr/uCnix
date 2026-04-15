#pragma once
#include <kernel/settings.h>
#include <board/board.h>

static inline void tiny_delay(int loops)
{
    for (volatile int i = 0; i < loops; i++) {
        __NOP();
    }
}

#ifdef BOARD_MCXA153

#define RS_SHIFT 8
#define EN_MASK (1 << 9)

static inline void write_port(struct hd44xxx_device* disp, uint8_t rs, uint8_t val)
{
    GPIO1->PCOR = 0x3FF; //clear the parralel bus
    GPIO1->PSOR = (rs << RS_SHIFT) | val;
    tiny_delay(1);
    GPIO1->PSOR = EN_MASK;
    tiny_delay(1);
    GPIO1->PCOR = EN_MASK;
    tiny_delay(1);
    GPIO1->PCOR = 0x3FF;
}

static inline void port_init()
{
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT1(1);
    
    for (int i = 0; i < 10; i++) {
        PORT1->PCR[i] = 0x00008000;
        GPIO1->PCOR = (1<<i);
        
        GPIO1->PDDR |= (1<<i);
    } 
}

#endif


