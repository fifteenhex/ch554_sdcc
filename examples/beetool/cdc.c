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
#else
#define cdc_stat_inc(which) do { } while(0)
#define cdc_stat_add(which, x) do { } while(0)
#endif

/* flags for cdc state */
static uint8_t flags = 0;
#define CDC_FLAG_CONFIGURED			(1 << 0)

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

int cdc_setup_class_irq()
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

static void cdc_setup_class(void)
{
	if(!(out_flags & (FLAG_CDC_OUT_SETLINECODING | FLAG_CDC_OUT_SETLINECODING)))
		return;

#ifdef CONFIG_CDC_ACM_DEBUG
	printf("setup class\r\n");
#endif

	flags |= CDC_FLAG_CONFIGURED;
	out_flags &= ~(FLAG_CDC_OUT_SETLINECODING | FLAG_CDC_OUT_SETLINECODING);

	usb_ep0_setup_send_response(0);
}

static void cdc_data_out(void)
{
	if(!(out_flags & FLAG_CDC_OUT_DATA_READY))
		return;

	cdc_stat_add(rx, cdc_data_len);

#ifdef CONFIG_CDC_ACM_DEBUG
	usb_print_epbuffer(2, out);

	printf("cdc data - len: %d\r\n", cdc_data_len);
	for(int i = 0; i < cdc_data_len; i++)
		printf("%02x ", epbuffer(2, out)[i]);
	printf("\r\n");
#endif

/*
 * if debugging is enabled sending data will
 * crap up the output so don't.
 */
#ifndef CONFIG_CDC_ACM_DEBUG
	for(int i = 0; i < cdc_data_len; i++) {
		while(uart_tx_full());
		uart_tx_push(epbuffer(2, out)[i]);
	}
#endif

	out_flags &= ~FLAG_CDC_OUT_DATA_READY;
	UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
}

static void cdc_data_in(void)
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

	while(uart_rx_have_data() && len < sizeof(epbuffer_ep2_in))
		epbuffer_ep2_in[len++] = uart_rx_pop();

	cdc_stat_add(tx, len);

	in_flags |= FLAG_CDC_IN_DATA_READY;
	UEP2_T_LEN = len;
	UEP2_CTRL = (UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
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
	cdc_setup_class();
	cdc_data_out();
	cdc_data_in();
	cdc_print_stats();
}
