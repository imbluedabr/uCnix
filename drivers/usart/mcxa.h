#pragma once
#include <drivers/usart.h>

void usart_mcxa_init(struct usart_device* usart, struct usart_desc* desc);
int usart_mcxa_readb(struct usart_device* usart);
int usart_mcxa_writeb(struct usart_device* usart, uint8_t val);
int usart_mcxa_ioctl(struct usart_device* usart, int op, void* arg);
void usart_mcxa_destroy(struct usart_device* usart);

