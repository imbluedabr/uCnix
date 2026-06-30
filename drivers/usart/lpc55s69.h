#pragma once
#include <drivers/usart.h>

void usart_lpc55s69_init(struct usart_device* usart, struct usart_desc* desc);
int usart_lpc55s69_readb(struct usart_device* usart);
int usart_lpc55s69_writeb(struct usart_device* usart, uint8_t val);
int usart_lpc55s69_ioctl(struct usart_device* usart, int op, void* arg);
void usart_lpc55s69_destroy(struct usart_device* usart);

