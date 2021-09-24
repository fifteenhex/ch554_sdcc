/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef USB_BUFFERS_H_
#define USB_BUFFERS_H_

#include <ch554_usb.h>
#include <stdint.h>
#include "config.h"

#include "cdc_proto.h"

#define USB_BUFFERS_ENDPOINT_SZ_UNIT	64

/* use this to access buffers when the ep is configurable */
#define epbuffer(which, dir) \
	epbuffer_ep##which##_##dir

/* endpoint 0 */
union ep0_composite {
	/*
	 * on page 47 of data sheet, the receive buffer
	 * need to be min(possible packet size+2,64)
	 */
	uint8_t buf[USB_BUFFERS_ENDPOINT_SZ_UNIT + 2];
	USB_SETUP_REQ setup_req;
};
extern __xdata union ep0_composite epbuffer_ep0_composite;
#define epbuffer_ep0 (epbuffer_ep0_composite.buf)
#define setupreq (epbuffer_ep0_composite.setup_req)

/* endpoint 1 */
#ifdef CONFIG_EP1_ENABLE
#if defined(CONFIG_EP1_IN) && defined(CONFIG_EP1_OUT)
#define EP1_BUFFER_SZ 128
#else
#define EP1_BUFFER_SZ 64
#endif

extern __xdata uint8_t  epbuffer_ep1[EP1_BUFFER_SZ];
#endif

/* endpoint 2 */
#ifdef CONFIG_EP2_ENABLE

struct ep2_composite {
#ifdef CONFIG_EP2_OUT
	union {
		uint8_t	out[USB_BUFFERS_ENDPOINT_SZ_UNIT];
		struct cdc_linecoding out_linecoding;
	};
#endif
#ifdef CONFIG_EP2_IN
	uint8_t in[USB_BUFFERS_ENDPOINT_SZ_UNIT];
#endif
};

extern __xdata struct ep2_composite epbuffer_ep2_composite;
#define epbuffer_ep2_out (epbuffer_ep2_composite.out)
#define epbuffer_ep2_in (epbuffer_ep2_composite.in)
#endif

/* endpoint 3 */
#ifdef CONFIG_EP3_ENABLE
#if defined(CONFIG_EP3_IN) && defined(CONFIG_EP3_OUT)
#define EP3_BUFFER_SZ 128
#else
#define EP3_BUFFER_SZ 64
#endif

extern __xdata uint8_t epbuffer_ep3[EP3_BUFFER_SZ];
#endif

/* endpoint 4 */
#ifdef CONFIG_EP4_ENABLE
#if defined(CONFIG_EP4_IN) && defined(CONFIG_EP4_OUT)
#define EP4_BUFFER_SZ 128
#else
#define EP4_BUFFER_SZ 64
#endif

extern __xdata uint8_t epbuffer_ep4[EP4_BUFFER_SZ];
#endif

#endif
