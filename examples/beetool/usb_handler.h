#ifndef __USB_HANDLER_H__
#define __USB_HANDLER_H__

#include <stdint.h>

#include "usb_descriptor.h"

#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)

void usb_interrupt(void);
void usb_configure(void);

#endif
