#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

#include <stdint.h>

#include "config.h"
#include "usb_descriptor.h"

#ifdef CONFIG_EP1_ENABLE
#ifdef CONFIG_EP1_IN
extern void usb_ep1_in(void);
#endif
#ifdef CONFIG_EP1_OUT
extern void usb_ep1_out(void);
#endif
#endif

#ifdef CONFIG_EP2_ENABLE

#if defined(CONFIG_EP2_IN) && defined(CONFIG_EP2_OUT)
#define EP2_BUFFER_SZ 128
#else
#define EP2_BUFFER_SZ 64
#endif

extern __xdata uint8_t  epbuffer_ep2[EP2_BUFFER_SZ];
#ifdef CONFIG_EP2_IN
extern void usb_ep2_in(void);
#endif
#ifdef CONFIG_EP2_OUT
extern void usb_ep2_out(void);
#endif
#endif

void usb_interrupt(void);
void usb_configure(void);
void usb_printstats(void);

#ifdef CONFIG_USB_PKTDBG
struct usb_stats {
	unsigned irqs;
	/* tx irqs */
	uint8_t tx;
	/* tx that where directed at an endpoint/direction that isn't enabled */
	uint8_t bad_ep;
	/* tog_ng */
	uint8_t tog_ng;
	/* unhandled */
	uint8_t unhandled;


	uint8_t ovr,rst, sus, spurious;

	/* ep0 */
	uint8_t in_ep0;
	uint8_t out_ep0;

	uint8_t setup;
	uint8_t setup_unhandled;

	/* others */
#ifdef CONFIG_EP1_IN
	uint8_t in_ep1;
#endif
#ifdef CONFIG_EP1_OUT
	uint8_t out_ep1;
#endif
#ifdef CONFIG_EP2_IN
	uint8_t in_ep2;
#endif
#ifdef CONFIG_EP2_OUT
	uint8_t out_ep2;
#endif
	uint8_t in_ep3, out_ep3;
};
extern __xdata struct usb_stats usb_stats;

#define usb_print_epbuffer(which)					\
do									\
{									\
	printf("endpoint %d buffer:\r\n", which);			\
	for(int i = 0; i < sizeof(epbuffer_ep##which); i++) {		\
		printf("%02x ", epbuffer_ep##which[i]);			\
		if((i + 1) % 16 == 0)					\
			printf("\r\n");					\
	}								\
	printf("\r\n");							\
} while(0)
#else
#define usb_print_epbuffer(which) do {} while(0)
#endif

#endif
