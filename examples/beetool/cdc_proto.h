/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef CDC_PROTO_H_
#define CDC_PROTO_H_

#define CDC_CLASS_REQUEST_SETLINECODING		0x20
#define CDC_CLASS_REQUEST_SETCONTROLLINESTATE	0x22

struct cdc_linecoding {
	uint8_t dwDTERate[4];
	uint8_t bCharFormat;
	uint8_t bParityType;
	uint8_t bDataBits;
};

#endif
