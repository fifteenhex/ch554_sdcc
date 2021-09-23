/*
 * BeeTool
 *
 * 2021 dgp <daniel@thingy.jp>
 */

#include <stdint.h>
#include <stdio.h>
#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

#include "config.h"
#include "usb_buffers.h"
#include "usb_handler.h"
#include "uart.h"

/* interrupts */
#include "uart_int.h"
#include "usb_int.h"

void usb_ep1_in(void)
{
	// No data to send anymore
	UEP1_T_LEN = 0;
	//Respond NAK by default
	UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
}

void usb_ep1_out(void)
{
	CH554UART1SendByte('1');
}

void main()
{
	CfgFsys();
	disconnectUSB();
	usb_configure();

	LED = 0;

	UART1Setup();
	uart_setup();

	uint8_t i = 0;

	int loop = 0;

	while (1) {
		cdc_main_loop();

		if (loop++ % 5000 == 0) {
			usb_printstats();
			LED = !LED;
			mDelaymS(500);
		}
	}
}
