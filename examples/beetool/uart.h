/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void uart_setup(void);

extern __xdata uint8_t uart_rx_buf[CONFIG_UART_BUFSZ];
extern __xdata uint8_t uart_tx_buf[CONFIG_UART_BUFSZ];

extern unsigned uart_rx_head, uart_rx_tail;
extern unsigned uart_tx_head, uart_tx_tail;

#define uart_rx_have_data() (uart_rx_tail != uart_rx_head)
#define uart_tx_have_data() (uart_tx_tail != uart_tx_head)

static inline uint8_t uart_rx_pop(void)
{
	uint8_t v = uart_rx_buf[uart_rx_tail];
	uart_rx_tail = (uart_rx_tail + 1) % sizeof(uart_rx_buf);
	return v;
}

static inline uint8_t uart_tx_pop(void)
{
	uint8_t v = uart_tx_buf[uart_tx_tail];
	uart_tx_tail = (uart_tx_tail + 1) % sizeof(uart_tx_buf);
	return v;
}

#define uart_tx_push(b)								\
	do { 									\
		uart_tx_buf[uart_tx_head] = b;					\
		uart_tx_head = (uart_tx_head + 1) % sizeof(uart_tx_buf);	\
	} while (0)

#define uart_rx_push(b)								\
	do { 									\
		uart_rx_buf[uart_rx_head] = b;					\
		uart_rx_head = (uart_rx_head + 1) % sizeof(uart_rx_buf);	\
	} while (0)

void uart_try_tx(void);

#endif
