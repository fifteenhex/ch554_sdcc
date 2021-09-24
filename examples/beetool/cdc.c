/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <stdio.h>
#include <ch554.h>
#include "cdc.h"
#include "cdc_proto.h"
#include "uart.h"
#include "usb_buffers.h"
#include "usb_handler.h"

#ifdef CONFIG_CDC_ACM_DEBUG
__xdata struct cdc_stats cdc_stats = { 0 };

#define cdc_stat_inc(which)		\
	do {				\
		cdc_stats.which++;	\
	} while(0)
#define cdc_stat_add(which, x)		\
	do {				\
		cdc_stats.which += x;	\
	} while(0)

#define	cdc_dbg printf
#else
#define	cdc_dbg
#define cdc_stat_inc(which) do { } while(0)
#define cdc_stat_add(which, x) do { } while(0)
#endif

#define cdc_linecoding_req	epbuffer(0, out_linecoding)
#define cdc_notification	epbuffer(1, in_notification)
#define cdc_data_out_buf	epbuffer(2, out)
#define cdc_data_in_buf		epbuffer(2, in)

/* flags for cdc state */
static uint8_t flags = 0;
#define CDC_FLAG_CONFIGURED			(1 << 0)
#define CDC_FLAG_SERIALSTATESYNC		(1 << 1)

/* flags set in interrupt */
static uint8_t out_flags = 0;
/* Data from the host is available */
#define FLAG_CDC_OUT_DATA_READY			(1 << 0)
#define FLAG_CDC_OUT_SETLINECODING		(1 << 1)
#define FLAG_CDC_OUT_SETCONTROLLINESTATE	(1 << 2)

/* flags set in loop */
static uint8_t in_flags = 0;
#define FLAG_CDC_IN_DATA_READY	1

static uint8_t cdc_data_len;

void cdc_notification_in_irq(void)
{
	// No data to send anymore
	UEP1_T_LEN = 0;
	//Respond NAK by default
	UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
}

void cdc_data_in_irq(void)
{
	in_flags &= ~FLAG_CDC_IN_DATA_READY;

	// No data to send anymore
	UEP2_T_LEN = 0;
	//Respond NAK by default
	UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
}


void cdc_data_out_irq(void)
{
	UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_NAK;
	cdc_data_len = USB_RX_LEN;
	out_flags |= FLAG_CDC_OUT_DATA_READY;
}

int cdc_setup_class_irq(void)
{
	switch(setupreq.bRequest){
	case CDC_CLASS_REQUEST_SETLINECODING:
		out_flags |= FLAG_CDC_OUT_SETLINECODING;
		return 0;
	case CDC_CLASS_REQUEST_SETCONTROLLINESTATE:
		out_flags |= FLAG_CDC_OUT_SETCONTROLLINESTATE;
		return 0;
	}

	return 1;
}

static inline void cdc_setup_class_setlinecoding(void)
{
	if(setupreq.wLengthL != LINECODING_SZ){
		cdc_dbg("Bad data size: %d\r\n", setupreq.wLengthL);
		return;
	}

	cdc_dbg("set line coding:\r\n");
	cdc_dbg("\tbaud rate: %lu\r\n", cdc_linecoding_req.dwDTERate);
	cdc_dbg("\tformat: %d\r\n", cdc_linecoding_req.bCharFormat);
	cdc_dbg("\tparity type: %d\r\n", cdc_linecoding_req.bParityType);
	cdc_dbg("\tdata bits: %d\r\n", cdc_linecoding_req.bDataBits);
}

static inline void cdc_setup_class_setcontrollinestate(void)
{
	if(setupreq.wLengthL != 0){
		cdc_dbg("Bad data size: %d\r\n", setupreq.wLengthL);
		return;
	}

	cdc_dbg("set control line state: %02x%02x\r\n",
				setupreq.wValueH, setupreq.wValueL);
}

static inline void cdc_setup_class(void)
{
	if(!(out_flags & (FLAG_CDC_OUT_SETLINECODING |
			FLAG_CDC_OUT_SETCONTROLLINESTATE)))
		return;

	if(out_flags & FLAG_CDC_OUT_SETLINECODING)
		cdc_setup_class_setlinecoding();

	if(out_flags & FLAG_CDC_OUT_SETCONTROLLINESTATE)
		cdc_setup_class_setcontrollinestate();

	flags |= CDC_FLAG_CONFIGURED;
	out_flags &= ~(FLAG_CDC_OUT_SETLINECODING | FLAG_CDC_OUT_SETCONTROLLINESTATE);

	usb_ep0_setup_send_response(0);
}

static inline void cdc_data_out(void)
{
	if(!(out_flags & FLAG_CDC_OUT_DATA_READY))
		return;

	cdc_stat_add(rx, cdc_data_len);

#ifdef CONFIG_CDC_ACM_DEBUG
	usb_print_epbuffer(2, out);

	printf("cdc data - len: %d\r\n", cdc_data_len);
	for(int i = 0; i < cdc_data_len; i++)
		printf("%02x ", cdc_data_out_buf[i]);
	printf("\r\n");
#endif

/*
 * if debugging is enabled sending data will
 * crap up the output so don't.
 */
#ifndef CONFIG_CDC_ACM_DEBUG
	for(int i = 0; i < cdc_data_len; i++) {
		while(uart_tx_full());
		uart_tx_push(cdc_data_out_buf[i]);
	}
#endif

	out_flags &= ~FLAG_CDC_OUT_DATA_READY;
	UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
}

static inline void cdc_data_in(void)
{
	int len = 0;

	/*
	 * We haven't been configured yet.
	 * For linux+minicom sending data to the host
	 * before being configured causes no data to
	 * ever arrive.
	 */
	if(!(flags & CDC_FLAG_CONFIGURED))
		return;

	/* interrupt hasn't cleared yet */
	if(in_flags & FLAG_CDC_IN_DATA_READY)
		return;

	if(!uart_rx_have_data())
		return;

#ifdef CONFIG_CDC_ACM_DEBUG
	printf("cdc: preparing tx\r\n");
#endif

	while(uart_rx_have_data() && len < sizeof(cdc_data_in_buf))
		cdc_data_in_buf[len++] = uart_rx_pop();

	cdc_stat_add(tx, len);

	in_flags |= FLAG_CDC_IN_DATA_READY;
	UEP2_T_LEN = len;
	UEP2_CTRL = (UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

static inline void cdc_send_notification(void)
{
	if(flags & CDC_FLAG_SERIALSTATESYNC)
		return;

	flags |= CDC_FLAG_SERIALSTATESYNC;

	cdc_notification.setup_req.bRequestType = 0xa1;
	cdc_notification.setup_req.bRequest = CDC_NOTIFICATION_SERIAL_STATE;
	cdc_notification.setup_req.wValueH = 0;
	cdc_notification.setup_req.wValueL = 0;
	cdc_notification.setup_req.wIndexH = 0;
	cdc_notification.setup_req.wIndexL = 0;
	cdc_notification.setup_req.wLengthH = 0;
	cdc_notification.setup_req.wLengthL = 2;
	cdc_notification.data = 0;

	UEP1_T_LEN = sizeof(cdc_notification);
	UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}
static inline void cdc_print_stats(void)
{
#ifdef CONFIG_CDC_ACM_DEBUG
	static int last = 0;
	int curr = cdc_stats.rx + cdc_stats.tx;

	if(last == curr)
		return;

	last = curr;

	printf("cdc: rx: %x\r\n", cdc_stats.rx);
	printf("cdc: tx: %x\r\n", cdc_stats.tx);
#endif
}

void cdc_main_loop()
{
	cdc_send_notification();
	cdc_setup_class();
	cdc_data_out();
	cdc_data_in();
	cdc_print_stats();
}
