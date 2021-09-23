/*
 * BeeTool
 *
 * 2021 dgp <daniel@thingy.jp>
 */

#include <stdint.h>
#include <stdio.h>
#include <ch554.h>
#include <ch554_usb.h>
#include <debug.h>

#include "config.h"
#include "usb_buffers.h"
#include "usb_handler.h"

// CRAP
//Bytes of received data on USB endpoint
volatile uint8_t USBByteCountEP1 = 0;

// for EP1 OUT double-buffering
volatile uint8_t EP1_buffs_avail = 2;
__bit EP1_buf_toggle = 0;
// CRAP


static uint8_t flags;
/* Data from the host is available */
#define FLAG_CDC_DATA_OUT	1

static void usb_irq(void) __interrupt (INT_NO_USB)
{
	usb_interrupt();
}

void usb_ep1_in(void)
{
	// No data to send anymore
	UEP1_T_LEN = 0;
	//Respond NAK by default
	UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;
}

void usb_ep1_out(void)
{
	CH554UART1SendByte('1');


		USBByteCountEP1 = USB_RX_LEN;
		if (USBByteCountEP1) {
			//Respond NAK. Let main change response after handling.
			UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;

			// double-buffering of DAP request packets
			//DAP_RxBuf = (__xdata uint8_t*) UEP1_DMA;
			EP1_buf_toggle = !EP1_buf_toggle;
			//if (EP1_buf_toggle)
			//    UEP1_DMA = (uint16_t) Ep1Buffer + 64;
			//else
			//    UEP1_DMA = (uint16_t) Ep1Buffer;

		}
}

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

void main()
{
	CfgFsys();
	disconnectUSB();
	usb_configure();

	LED = 0;

	UART1Setup();

	uint8_t i = 0;

	int loop = 0;

	while (1) {
		cdc_data_out();

		if (loop++ % 5000 == 0) {
			usb_printstats();
			LED = !LED;
			mDelaymS(500);
		}
	}
}
