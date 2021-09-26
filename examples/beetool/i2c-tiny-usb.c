/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <stdio.h>
#include <stdint.h>
#include <ch554.h>
#include <ch554_usb.h>

#include "usb_buffers.h"
#include "usb_handler.h"
#include "i2c-tiny-usb.h"

#define CMD_ECHO		0
#define CMD_GET_FUNC		1
#define CMD_SET_DELAY		2
#define CMD_GET_STATUS		3

#define CMD_I2C_IO		4
#define CMD_I2C_IO_BEGIN	(1<<0)
#define CMD_I2C_IO_END		(1<<1)

static uint8_t flags = 0;
#define FLAG_GET_ECHO	(1 << 0)
#define FLAG_GET_FUNC	(1 << 1)
#define FLAG_SET_DELAY	(1 << 2)

static int xxx = 0;

int i2c_tiny_setup_vendor_irq(void) {
	switch(setupreq.bRequest){
		case CMD_GET_FUNC:
			flags |= FLAG_GET_FUNC;
			return 0;
		case CMD_SET_DELAY:
			xxx++;
			flags |= FLAG_SET_DELAY;
			return 0;
	}
	return 1;
}

static void i2c_tiny_get_func(void)
{
	if(!(flags & FLAG_GET_FUNC))
		return;

	epbuffer_i2c_tiny_func = 0;

	flags &= ~FLAG_GET_FUNC;
	usb_ep0_setup_send_response(sizeof(epbuffer_i2c_tiny_func));
}

static void i2c_tiny_set_delay(void)
{
	if(!(flags & FLAG_SET_DELAY))
		return;

	flags &= ~FLAG_SET_DELAY;

	usb_ep0_setup_send_response(0);
}

void i2c_tiny_main(void)
{
	static int last = 0;

	i2c_tiny_get_func();
	i2c_tiny_set_delay();

	if(last != xxx) {
		printf("i2c-tiny: %d\r\r", xxx);
	}

	last = xxx;
}
