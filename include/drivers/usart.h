#pragma once
#include <kernel/device.h>

typedef enum : uint8_t {
    MCXA_LPUART
} usart_type_e;

struct usart_desc {
    void* base;
    uint8_t baud; //baudrate is computed by 300 * (2 << n)
    uint8_t irq;
    usart_type_e type;
};

#define USART_CALC_BAUD(N) (300 * (1 << N))

#define USART_TX_FIFO_LEN 16
#define USART_TX_FIFO_MSK (USART_TX_FIFO_LEN - 1)
#define USART_RX_FIFO_LEN 16
#define USART_RX_FIFO_MSK (USART_RX_FIFO_LEN - 1)

struct usart_device {
    struct device base;
    void* usart_base;
    char tx_fifo[USART_TX_FIFO_LEN];
    char rx_fifo[USART_RX_FIFO_LEN];
    uint8_t tx_head;
    uint8_t tx_tail;
    uint8_t rx_head;
    uint8_t rx_tail;
    uint32_t bytes_transfered;
};

void usart_init();
struct device* usart_create(uint8_t* minor, void* desc);
int usart_destroy(struct device* dev);
int usart_ioctl(struct device* dev, int op, void* arg);
int usart_readb(struct device* dev);
int usart_writeb(struct device* dev, char val);
void usart_update(struct device* dev);

struct usart_impl {
    void (*init)(struct usart_device* usart, struct usart_desc* desc);
    int (*readb)(struct usart_device* usart);
    int (*writeb)(struct usart_device* usart, uint8_t val);
    int (*ioctl)(struct usart_device* usart, int op, void* arg);
    void (*destroy)(struct usart_device* usart);
};

