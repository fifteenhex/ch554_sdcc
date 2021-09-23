/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <stdio.h>
#include <ch554.h>
#include "cdc.h"
#include "cdc_proto.h"
#include "usb_buffers.h"
#include "usb_handler.h"

__xdata struct cdc_stats cdc_stats = { 0 };
#define cdc_stat_inc(which)		\
	do {				\
		cdc_stats.which++;	\
	} while(0)
#define cdc_stat_add(which, x)		\
	do {				\
		cdc_stats.which += x;	\
	} while(0)


/* flags set in interrupt */
static uint8_t out_flags = 0;
/* Data from the host is available */
#define FLAG_CDC_OUT_DATA_READY	1

/* flags set in loop */
static uint8_t in_flags = 0;
#define FLAG_CDC_IN_DATA_READY	1

void usb_ep2_in(void)
{
	in_flags &= ~FLAG_CDC_IN_DATA_READY;

	// No data to send anymore
	UEP2_T_LEN = 0;
	//Respond NAK by default
	UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
}

static uint8_t cdc_data_len;
void usb_ep2_out(void)
{
	UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_NAK;
	cdc_data_len = USB_RX_LEN;
	out_flags |= FLAG_CDC_OUT_DATA_READY;
}

int cdc_setup_class()
{
	printf("cc %02x\r\n", setupreq.bRequest);

	switch(setupreq.bRequest){
	case CDC_CLASS_REQUEST_SETLINECODING:
	case CDC_CLASS_REQUEST_SETCONTROLLINESTATE:
		usb_ep0_setup_send_response(0);
		return 0;
	}

	return 1;
}

static void cdc_data_out(void)
{
	if(!(out_flags & FLAG_CDC_OUT_DATA_READY))
		return;

	cdc_stat_add(rx, cdc_data_len);

#ifdef CONFIG_CDC_DEBUG
	usb_print_epbuffer(2);

	printf("cdc data - len: %d\r\n", cdc_data_len);
	for(int i = 0; i < cdc_data_len; i++)
		printf("%02x ", epbuffer_ep2[i]);
	printf("\r\n");
#endif

/*
 * if debugging is enabled sending data will
 * crap up the output so don't.
 */
#ifndef CONFIG_CDC_DEBUG
	for(int i = 0; i < cdc_data_len; i++) {
		printf("%c", epbuffer_ep2[i]);
	}
#endif

	out_flags &= ~FLAG_CDC_OUT_DATA_READY;
	UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
}

static void cdc_data_in(void)
{
	/* interrupt hasn't cleared yet */
	if(in_flags & FLAG_CDC_IN_DATA_READY)
		return;

	cdc_stat_add(tx, cdc_data_len);

#ifdef CONFIG_CDC_DEBUG
	printf("cdc: preparing tx\r\n");
#endif

	in_flags |= FLAG_CDC_IN_DATA_READY;

	epbuffer_ep2_in[0] = 'B';
	UEP2_T_LEN = 1;
	UEP2_CTRL = (UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

static inline void cdc_print_stats(void)
{
	static int last = 0;
	int curr = cdc_stats.rx + cdc_stats.tx;

	if(last == curr)
		return;

	last = curr;

	printf("cdc: rx: %x\r\n", cdc_stats.rx);
	printf("cdc: tx: %x\r\n", cdc_stats.tx);
}

void cdc_main_loop()
{
	cdc_data_out();
	if((cdc_stats.rx + 1) % 10 == 0)
		cdc_data_in();
	cdc_print_stats();
}
