/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <ch554.h>

#include <debug.h>

#include "config.h"
#include "uart.h"

__xdata uint8_t uart_rx_buf[CONFIG_UART_BUFSZ];
__xdata uint8_t uart_tx_buf[CONFIG_UART_BUFSZ];

unsigned uart_rx_head = 0, uart_rx_tail = 0;
unsigned uart_tx_head = 0, uart_tx_tail = 0;

void uart_setup() {
	IE_UART1 = 1;
}

void uart_try_tx(void)
{
	if(U1TI) {
		//if(uart_tx_have_data()) {
			SBUF1 = uart_tx_pop();
			U1TI = 0;
		//}
	}
}

/* allow debug output to be mixed in */
int putchar(int ch)
{
	uart_tx_push(ch);
	CH554UART1SendByte(uart_tx_pop());
	//uart_try_tx();
	return 0;
}

/* we don't do getchar */
int getchar(void)
{
	return 0;
}
