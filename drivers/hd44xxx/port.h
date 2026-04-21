#pragma once
#include <kernel/settings.h>
#include <board/board.h>
#include <drivers/hd44xxx.h>

static inline void tiny_delay(int loops)
{
    for (volatile int i = 0; i < loops; i++) {
        __NOP();
    }
}

#ifdef BOARD_MCXA153

#define RS_SHIFT 8
#define RW_MASK (1 << 10)
#define EN_MASK (1 << 9)

static inline void write_port(struct hd44xxx_device* disp, uint8_t rs, uint8_t val)
{
    GPIO1->PCOR = 0x7FF; //clear the parralel bus
    GPIO1->PSOR = (rs << RS_SHIFT) | val;
    tiny_delay(10);
    GPIO1->PSOR = EN_MASK;
    tiny_delay(20);
    GPIO1->PCOR = EN_MASK;
    tiny_delay(10);
    GPIO1->PCOR = 0x7FF;
}

static inline uint8_t read_port(struct hd44xxx_device* disp, uint8_t rs)
{
    uint8_t recieved;
    GPIO1->PCOR = 0x7FF; //clear the parralel bus
    GPIO1->PDDR &= ~0xFF; //set data to input
    GPIO1->PSOR = (rs << RS_SHIFT) | RW_MASK;
    tiny_delay(10);
    GPIO1->PSOR = EN_MASK;
    tiny_delay(20);
    recieved = GPIO1->PDIR & 0xFF;
    tiny_delay(20);
    GPIO1->PCOR = EN_MASK;
    tiny_delay(10);
    GPIO1->PDDR |= 0xFF; //set data to output
    GPIO1->PCOR = 0x7FF;

    return recieved;
}

static inline void port_init()
{
    MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_PORT1(1);
    
    for (int i = 0; i < 11; i++) {
        PORT1->PCR[i] = 0x00008000 | PORT_PCR_IBE(1);
        GPIO1->PCOR = (1<<i);
        
        GPIO1->PDDR |= (1<<i);
    } 
}

#endif


