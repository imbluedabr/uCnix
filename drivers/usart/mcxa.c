#include "mcxa.h"
#include <board/board.h>
#include <kernel/interrupt.h>


void usart_mcxa_interrupt()
{
    struct usart_device* usart = get_current_handler_struct();

    volatile LPUART_Type* lpuart = usart->usart_base;
    
    if (usart->tx_tail != usart->tx_head) {
        uint8_t tmp = (usart->tx_tail + 1) & USART_TX_FIFO_MSK;
        usart->tx_tail = tmp;
        lpuart->DATA = usart->tx_fifo[tmp];
    } else {
        lpuart->CTRL &= ~LPUART_CTRL_TIE_MASK;
    }
    char c = lpuart->DATA;
    uint8_t tmp = (usart->rx_head + 1) & USART_RX_FIFO_MSK;
    if (usart->rx_tail != usart->rx_head) {
        usart->rx_fifo[tmp] = c;
        usart->rx_head = tmp;
    }

    NVIC_ClearPendingIRQ(get_current_interrupt());
}

void usart_mcxa_init(struct usart_device* usart, struct usart_desc* desc)
{
    volatile LPUART_Type* lpuart = desc->base;
    MRCC0->MRCC_LPUART0_CLKSEL = MRCC_MRCC_LPUART0_CLKSEL_MUX(2);
    MRCC0->MRCC_LPUART0_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);
    
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_LPUART0(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_CC0_PORT0(1);

    PORT0->PCR[2] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT0->PCR[3] = PORT_PCR_LK(1) | PORT_PCR_MUX(2);
    
    register_interrupt(desc->irq, usart, usart_mcxa_interrupt);
    NVIC_SetPriority(desc->irq, 3);
    NVIC_ClearPendingIRQ(desc->irq);
    NVIC_EnableIRQ(desc->irq);

    lpuart->BAUD = LPUART_BAUD_OSR(0b01111) | LPUART_BAUD_SBR(CLK_FRO_48MHZ / (USART_CALC_BAUD(desc->baud)* 16));
    lpuart->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK | LPUART_CTRL_RIE_MASK;
}

int usart_mcxa_readb(struct usart_device* usart)
{   
    if (usart->rx_tail == usart->rx_head) {
        return -1;
    }

    uint8_t tmp = (usart->rx_tail + 1) & USART_RX_FIFO_MSK;
    usart->rx_tail = tmp;

    return usart->rx_fifo[tmp];
}

int usart_mcxa_writeb(struct usart_device* usart, uint8_t val)
{
    uint8_t tmp = (usart->tx_head + 1) & USART_TX_FIFO_MSK;
    if (tmp == usart->tx_tail) {
        return -1;
    }
    usart->tx_head = tmp;
    usart->tx_fifo[tmp] = val;
    volatile LPUART_Type* lpuart = usart->usart_base;
    lpuart->CTRL |= LPUART_CTRL_TIE_MASK;
    return 0;

}

int usart_mcxa_ioctl(struct usart_device* usart, int op, void* arg)
{
    return -1;
}

void usart_mcxa_destroy(struct usart_device* usart)
{

}


