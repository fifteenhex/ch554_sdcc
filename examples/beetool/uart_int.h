#ifndef UART_INT_H_
#define UART_INT_H_

#include "uart.h"

static void uart1_irq(void) __interrupt (INT_NO_UART1)
{
	//uart1_try_tx();

	if (U1RI) {
		U1RI = 0;
		uart_rx_push(SBUF1);
	}
}

#endif
