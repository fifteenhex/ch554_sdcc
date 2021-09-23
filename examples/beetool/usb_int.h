/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef USB_INT_H_
#define USB_INT_H_

#include "usb_handler.h"

static void usb_irq(void) __interrupt (INT_NO_USB)
{
	usb_interrupt();
}

#endif
