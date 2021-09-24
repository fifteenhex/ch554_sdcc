/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <ch554.h>

#include <debug.h>

#include "config.h"
#include "uart.h"

__xdata uint8_t uart_flags = UART_FLAG_IDLE;

__xdata uint8_t uart_rx_buf[CONFIG_UART_BUFSZ];
__xdata uint8_t uart_tx_buf[CONFIG_UART_BUFSZ];

unsigned uart_rx_head = 0, uart_rx_tail = 0;
unsigned uart_tx_head = 0, uart_tx_tail = 0;

void uart_setup()
{
	IE_UART1 = 1;
}

/*
 * allow debug output to be mixed in.
 * printf must not be called from an interrupt!
 */
int putchar(int ch)
{
	/*
	 * long printf lines will fill the buffer
	 * and then get mashed up. So if the buffer is full
	 * waiting before pushing into it.
	 */
	while(uart_tx_full());

	uart_tx_push(ch);
	return 0;
}

/* we don't do getchar */
int getchar(void)
{
	return 0;
}
