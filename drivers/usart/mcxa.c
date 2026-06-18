#include "mcxa.h"
#include <board/board.h>
#include <kernel/interrupt.h>
#include <lib/kprint.h>
#include <drivers/usart.h>


static void* lpuart0_init()
{
    MRCC0->MRCC_LPUART0_CLKSEL = MRCC_MRCC_LPUART0_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART0_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);
    
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT0(1);

    PORT0->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT0->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);
    
    return LPUART0;
}

static void* lpuart1_init()
{
    MRCC0->MRCC_LPUART1_CLKSEL = MRCC_MRCC_LPUART1_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART1_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART1(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT2(1);
    
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART1(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT2(1);

    PORT2->PCR[12] = PORT_PCR_LK(1) | PORT_PCR_MUX(3) | PORT_PCR_IBE(1);
    PORT2->PCR[13] = PORT_PCR_LK(1) | PORT_PCR_MUX(3);

    return LPUART1;
}

static void* lpuart2_init()
{
    MRCC0->MRCC_LPUART2_CLKSEL = MRCC_MRCC_LPUART2_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART2_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART2(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);
    
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);

    PORT1->PCR[4] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT1->PCR[5] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);

    return LPUART2;
}

typedef void* (*lpuart_init_t)();
const lpuart_init_t initializers[] = {
    lpuart0_init,
    lpuart1_init,
    lpuart2_init
};

void usart_mcxa_interrupt()
{
    struct usart_device* usart = get_current_handler_struct();

    volatile LPUART_Type* lpuart = usart->usart_base;
    
    if (lpuart->STAT & LPUART_STAT_RDRF_MASK) {
        char c = lpuart->DATA;
        //lpuart->DATA = 'B';
        uint8_t tmp = usart->rx_head;
        usart->rx_fifo[tmp] = c;
        usart->rx_head = (tmp + 1) & USART_RX_FIFO_MSK;
        return;
    }
    
    if (usart->tx_tail != usart->tx_head) {
        uint8_t tmp = (usart->tx_tail + 1) & USART_TX_FIFO_MSK;
        usart->tx_tail = tmp;
        lpuart->DATA = usart->tx_fifo[tmp];
    } else {
        lpuart->CTRL &= ~LPUART_CTRL_TIE_MASK;
    }
}

void usart_mcxa_init(struct usart_device* usart, struct usart_desc* desc)
{
    volatile LPUART_Type* lpuart = initializers[desc->uart_num]();
    usart->usart_base = (void*) lpuart;

    register_interrupt(desc->irq, usart, usart_mcxa_interrupt);
    NVIC_SetPriority(desc->irq, 3);
    NVIC_ClearPendingIRQ(desc->irq);
    NVIC_EnableIRQ(desc->irq);

    lpuart->BAUD = LPUART_BAUD_OSR(0b01111) | LPUART_BAUD_SBR(CLK_FRO_48MHZ / (usart_baud_rates[desc->baud]*16));
    lpuart->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK | LPUART_CTRL_RIE_MASK;
}

int usart_mcxa_readb(struct usart_device* usart)
{
    __disable_irq();
    if (usart->rx_tail == usart->rx_head) {
        __enable_irq();
        return -1;
    }
    uint8_t tmp = usart->rx_tail;
    usart->rx_tail = (tmp + 1) & USART_RX_FIFO_MSK;
    __enable_irq();
    //kputc('A');
    return usart->rx_fifo[tmp];
}

int usart_mcxa_writeb(struct usart_device* usart, uint8_t val)
{
    __disable_irq();
    uint8_t tmp = (usart->tx_head + 1) & USART_TX_FIFO_MSK;
    if (tmp == usart->tx_tail) {
        __enable_irq();
        return -1;
    }
    usart->tx_head = tmp;
    usart->tx_fifo[tmp] = val;
    volatile LPUART_Type* lpuart = usart->usart_base;
    lpuart->CTRL |= LPUART_CTRL_TIE_MASK;
    __enable_irq();
    return 0;
}

int usart_mcxa_ioctl(struct usart_device* usart, int op, void* arg)
{
    return -1;
}

void usart_mcxa_destroy(struct usart_device* usart)
{

}


