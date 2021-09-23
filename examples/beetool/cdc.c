/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include <stdio.h>
#include "cdc.h"
#include "cdc_proto.h"
#include "usb_buffers.h"
#include "usb_handler.h"

#include <ch554.h>

static uint8_t flags;
/* Data from the host is available */
#define FLAG_CDC_DATA_OUT	1

void usb_ep2_in(void)
{
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
	flags |= FLAG_CDC_DATA_OUT;
}

int cdc_setup_class()
{
	switch(setupreq.bRequest){
	case CDC_CLASS_REQUEST_SETCONTROLLINESTATE:
		printf("yay!\n");
		return 0;
	}
	return 1;
}

static void cdc_data_out(void)
{
	if(!(flags & FLAG_CDC_DATA_OUT))
		return;

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

	flags &= ~FLAG_CDC_DATA_OUT;
	UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
}

void cdc_main_loop()
{
	cdc_data_out();
}
