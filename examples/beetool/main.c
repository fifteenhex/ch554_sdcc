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
#include "usb_handler.h"

//Bytes of received data on USB endpoint
volatile uint8_t USBByteCountEP1 = 0;

// for EP1 OUT double-buffering 
volatile uint8_t EP1_buffs_avail = 2;
__bit EP1_buf_toggle = 0;

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

	// Discard unsynchronized packets
	if (U_TOG_OK) {
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
}

void usb_ep2_in(void)
{
	UEP2_T_LEN = 0;                     // No data to send anymore
	UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Respond NAK by default
}

void usb_ep2_out(void)
{
}


void main()
{
	CfgFsys();
	disconnectUSB();
	usb_configure();

	LED = 0;

	UART1Setup();


	//LED = 1;

	uint8_t i = 0;


	while (1) {
		//printf("shizzle\n");
		//printf("main loop\n");
#if 0
		uint8_t response_len;
        // process if a DAP packet is received, and TxBuf is empty
        // save ByteCountEP1?
        if (USBByteCountEP1 && !UEP2_T_LEN) {
            __xdata uint8_t* RxPkt = DAP_RxBuf;
            if (--EP1_buffs_avail) {
                USBByteCountEP1 = 0 ;
                // Rx another packet while DAP_Thread runs
                UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
            }

            //UEP2_T_LEN = response_len;
            // enable interrupt IN response
            UEP2_T_LEN = 64;            // hangs on Windoze < 64
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;

            // enable receive
            EP1_buffs_avail++;
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
        }
#endif

		mDelaymS(5000);
		usb_printstats();
    }
}
