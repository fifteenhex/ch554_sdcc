/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ch554.h>
#include <ch554_usb.h>

#include "usb_buffers.h"
#include "usb_handler.h"
#include "i2c-tiny-usb.h"

#include "linux-i2c.h"

#define CMD_ECHO		0
#define CMD_GET_FUNC		1
#define CMD_SET_DELAY		2
#define CMD_GET_STATUS		3

#define CMD_I2C_IO		4
#define CMD_I2C_IO_BEGIN	(1<<0)
#define CMD_I2C_IO_END		(1<<1)

#define STATUS_IDLE		0
#define STATUS_ADDRESS_ACK	1
#define STATUS_ADDRESS_NAK	2

static uint8_t flags = 0;
#define FLAG_GET_ECHO		(1 << 0)
#define FLAG_GET_FUNC		(1 << 1)
#define FLAG_SET_DELAY		(1 << 2)
#define FLAG_GET_STATUS		(1 << 3)
#define FLAG_DO_IO		(1 << 4)
#define FLAG_DO_IO_START	(1 << 5)
#define FLAG_DO_IO_END		(1 << 6)

int i2c_tiny_setup_vendor_irq(void) {
	switch(setupreq.bRequest){
		case CMD_GET_FUNC:
			flags |= FLAG_GET_FUNC;
			return 0;
		case CMD_SET_DELAY:
			flags |= FLAG_SET_DELAY;
			return 0;
		case CMD_GET_STATUS:
			flags |= FLAG_GET_STATUS;
			return 0;
		case CMD_I2C_IO:
		case (CMD_I2C_IO | CMD_I2C_IO_BEGIN):
		case (CMD_I2C_IO | CMD_I2C_IO_END):
		case (CMD_I2C_IO | CMD_I2C_IO_BEGIN | CMD_I2C_IO_END):
			flags |= FLAG_DO_IO;
			return 0;
	}
	return 1;
}

static void i2c_tiny_get_func(void)
{
	if(!(flags & FLAG_GET_FUNC))
		return;

	epbuffer_i2c_tiny_func = I2C_FUNC_I2C |
				I2C_FUNC_SMBUS_EMUL;

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

static void i2c_tiny_get_status(void)
{
	if(!(flags & FLAG_GET_STATUS))
		return;

	epbuffer_i2c_tiny_status = STATUS_IDLE;

	flags &= ~FLAG_GET_STATUS;

	usb_ep0_setup_send_response(sizeof(epbuffer_i2c_tiny_status));
}

static void i2c_tiny_do_io(void)
{
	if(!(flags & FLAG_DO_IO))
		return;

	bool read = setupreq.wValue & I2C_M_RD;

	printf("do io\r\n");

	// address setupreq.wIndex;
	// read data epbuffer_ep0
	// flags setupreq.wValue;

	if (read) {

	}
	else {

	}

	flags &= ~FLAG_DO_IO;

	usb_ep0_setup_send_response(read ? setupreq.wLength : 0);
}

void i2c_tiny_main(void)
{
	i2c_tiny_get_func();
	i2c_tiny_set_delay();
	i2c_tiny_get_status();
	i2c_tiny_do_io();
}
