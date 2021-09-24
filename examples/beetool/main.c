/* SPDX-License-Identifier: GPL-3.0 */
/*
 * BeeTool
 *
 * 2021 Daniel Palmer <daniel@thingy.jp>
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

void main()
{
	CfgFsys();
	disconnectUSB();
	usb_configure();

	LED = 0;

	uart_setup();

	uint8_t i = 0;

	int loop = 0;

	while (1) {
		cdc_main_loop();

		if (loop++ % 5000 == 0) {
			usb_printstats();
			LED = !LED;
			//mDelaymS(500);
		}
	}
}
