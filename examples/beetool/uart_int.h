#ifndef UART_INT_H_
#define UART_INT_H_

#include "uart.h"

static void uart1_irq(void) __interrupt (INT_NO_UART1)
{
	if (U1TI){
		U1TI = 0;
		if(uart_tx_have_data())
			SBUF1 = uart_tx_pop();
		else
			uart_flags |= UART_FLAG_IDLE;
	}

	if (U1RI) {
		U1RI = 0;
		uart_rx_push(SBUF1);
	}
}

#endif
