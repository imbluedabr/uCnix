#include <drivers/usart.h>
#include <lib/kmalloc.h>
#include <lib/kprint.h>
#include <kernel/settings.h>
#include <kernel/majors.h>
#include <kernel/proc.h>
#include <kernel/interrupt.h>
#include <uapi/sys/errno.h>
#include <board/lpc55s69/board.h>

const int usart_baud_rates[11] = {
    110,
    300,
    1200,
    2400,
    4800,
    9600,
    19200,
    28800,
    38400,
    57600,
    115200
};

static struct dev_ops usart_ops = {
	.ll_read = usart_read,
	.ll_write = usart_write
};

struct device_driver usart_driver = {
    .probe = usart_probe,
    .name = "usart",
    .bus_accept = BUS_MMIO
};

static inline int readb(struct usart_device* usart)
{
    if (USART0->FIFOSTAT & USART_FIFOSTAT_RXNOTEMPTY_MASK) {
        char c = USART0->FIFORD;
        if (c == 'h') return -1;
        return c;
    }
    return -1;
}

ssize_t usart_read(struct device* dev, void* buff, size_t count)
{
    struct usart_device* usart = (struct usart_device*) dev;
    uint32_t i;
    for (i = 0; i < count; i++) {
        int c;
        if ((c = readb(usart)) < 0) {
            break;
        }
        ((uint8_t*) buff)[i] = c;
    }
    if (!i) return -EAGAIN;
    return i;
}

static inline int writeb(struct usart_device* usart, uint8_t val)
{
    if (!(USART0->FIFOSTAT & USART_FIFOSTAT_TXNOTFULL_MASK)) return -1;
    USART0->FIFOWR = val;
    return 0;
}

ssize_t usart_write(struct device* dev, const void* buff, size_t count)
{
    struct usart_device* usart = (struct usart_device*) dev;
    uint32_t i;
    for (i = 0; i < count; i++) {
        if (writeb(usart, ((uint8_t*) buff)[i]) < 0) {
            break;
        }
    }
    if (!i) return -EAGAIN;
    return i;   
}


void usart_init()
{
    register_driver(USART_MAJOR, &usart_driver);
}

#define CALC_BAUD(BAUD) (12000000/(16*BAUD) - 1)
struct device* usart_probe(struct bus_device* parent, const void* descriptor)
{
    struct usart_device* dev = kzalloc(sizeof(struct usart_device));
    if (!dev) return NULL;
    const struct mmio_bus_desc* desc = descriptor;
    dev->usart_base = desc->base;    
    dev->base.ops = &usart_ops;

    //enable ioconn module
    SYSCON->AHBCLKCTRLX[0] |= SYSCON_AHBCLKCTRL0_IOCON(1);

    //enable flexcomm 0
    SYSCON->AHBCLKCTRLX[1] |= SYSCON_AHBCLKCTRL1_FC0(1);
    SYSCON->PRESETCTRLCLR[1] = SYSCON_PRESETCTRL1_FC0_RST(1);
    SYSCON->FCCLKSELX[0] = SYSCON_FCCLKSEL0_SEL(2); //select 12 MHz clock

    //set the pin function
    IOCON->PIO[0][24] |= IOCON_PIO_FUNC(1); //pin 24 is rx
    IOCON->PIO[0][25] |= IOCON_PIO_FUNC(1); //pin 25 is tx
    
    //setup flexcom0 as a usart
    FLEXCOMM0->PSELID = FLEXCOMM_PSELID_PERSEL(1) | FLEXCOMM_PSELID_LOCK(1);
    

    //setup usart
    USART0->CFG = USART_CFG_ENABLE(1) | USART_CFG_DATALEN(1); //8 data bits, no parity, 1 stop bit
    USART0->BRG = CALC_BAUD(usart_baud_rates[115200]);
    USART0->OSR = USART_OSR_OSRVAL(15); //16x oversampling
    USART0->FIFOCFG = USART_FIFOCFG_ENABLETX(1) | USART_FIFOCFG_ENABLERX(1) | USART_FIFOCFG_EMPTYTX(1) | USART_FIFOCFG_EMPTYRX(1); //enable fifo's and reset the tx/rx fifo


    return &dev->base;
}




