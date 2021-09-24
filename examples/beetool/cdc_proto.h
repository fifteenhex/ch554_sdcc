/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef CDC_PROTO_H_
#define CDC_PROTO_H_

#include <stdint.h>
#include <ch554_usb.h>

#define CDC_CLASS_REQUEST_SETLINECODING		0x20
#define CDC_CLASS_REQUEST_GETLINECODING		0x21
#define CDC_CLASS_REQUEST_SETCONTROLLINESTATE	0x22

#define CDC_NOTIFICATION_SERIAL_STATE		0x20

struct cdc_linecoding {
	USB_SETUP_REQ setup_req;
	uint32_t dwDTERate;
	uint8_t bCharFormat;
	uint8_t bParityType;
	uint8_t bDataBits;
};

struct cdc_notification_serial_state {
	USB_SETUP_REQ setup_req;
	uint16_t data;
};

#define LINECODING_SZ (sizeof(struct cdc_linecoding) - sizeof(USB_SETUP_REQ))

#endif
