#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

#include <stdint.h>

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

struct usb_stats {
	unsigned irqs;
	uint8_t ovr, tx, rst, sus, spurious;

	/* ep0 */
	uint8_t in_ep0;
	uint8_t out_ep0;

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

	/* tx that where directed at an endpoint/direction that isn't enabled */
	uint8_t bad_ep;
};
extern __xdata struct usb_stats usb_stats;

#endif
