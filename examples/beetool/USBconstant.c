/* Descriptors for beetool */

#include <stdint.h>
#include <ch554_usb.h>
#include "config.h"

/* Device descriptor */
__code USB_DEV_DESCR DevDesc = {
	.bLength = 18,
	.bDescriptorType = USB_DESCR_TYP_DEVICE,
	.bcdUSBH = 0x01, .bcdUSBL = 0x10,
	// interface will define class
	.bDeviceClass =  0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = DEFAULT_ENDP0_SIZE,
	.idVendorH = 0x12, .idVendorL = 0x09,
	.idProductH = 0xC5, .idProductL = 0x5D,
	.bcdDeviceH = 0x01, .bcdDeviceL = 0x00,
	// string descriptors
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1
};

__code uint8_t CfgDescLen = sizeof(CfgDesc);

__code struct {
	USB_CFG_DESCR config;
	/* cdc */
	USB_ITF_DESCR cdc_control_interface;
	USB_HEADERFUNCTIONAL_DESCR cdc_header_functional;
	USB_ACMFUNCTIONAL_DESCR cdc_acm_functional;
	USB_UNIONFUNCTIONAL_DESCR cdc_union_functional;
	USB_CALLMANAGEMENTFUNCTIONAL_DESCR cdc_call_management_functional;
	USB_ENDP_DESCR ep3IN;
	USB_ITF_DESCR cdc_data_interface;
	USB_ENDP_DESCR ep4IN;
	USB_ENDP_DESCR ep4OUT;
	/* i2c */
	USB_ITF_DESCR i2c_interface;
	USB_ENDP_DESCR ep1IN;
	USB_ENDP_DESCR ep1OUT;
} CfgDesc = {
	.config = {
		.bLength = sizeof(USB_CFG_DESCR),
		.bDescriptorType = USB_DESCR_TYP_CONFIG,
		.wTotalLengthL = (sizeof(CfgDesc) & 0xff),
		.wTotalLengthH = ((sizeof(CfgDesc) >> 8) & 0xff),
		.bNumInterfaces = 3,
		.bConfigurationValue = 1,
		/* string index */
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		 /* 2mA units */
		.MaxPower = 50
	},
	/* cdc control */
	.cdc_control_interface = {
		.bLength = sizeof(USB_ITF_DESCR),
		.bDescriptorType = USB_DESCR_TYP_INTERF,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_DEV_CLASS_COMMUNIC,
		.bInterfaceSubClass = USB_DEV_SUBCLASS_CDC_ACM,
		.bInterfaceProtocol = 0,
		// no interface string
		.iInterface = 0,
	},
	.cdc_header_functional = HEADERFUNCTIONAL_DESCR(0),
	.cdc_acm_functional = ACMFUNCTIONAL_DESCR(0x2, 0xf),
	.cdc_union_functional = UNIONFUNCTIONAL_DESCR(0, 1),
	.cdc_call_management_functional = CALLMANAGEMENTFUNCTIONAL_DESCR(1),
	.ep3IN = {
		.bLength = sizeof(USB_ENDP_DESCR),
		.bDescriptorType = USB_DESCR_TYP_ENDP,
		// EP3 IN
		.bEndpointAddress = 0x83,
		.bmAttributes = USB_ENDP_TYPE_INTER,
		.wMaxPacketSizeL = HID_PKT_SIZ,
		.wMaxPacketSizeH = 0,
		// poll every 1 ms
		.bInterval = 1,
	},
	/* cdc data */
	.cdc_data_interface = {
		.bLength = sizeof(USB_ITF_DESCR),
		.bDescriptorType = USB_DESCR_TYP_INTERF,
		.bInterfaceNumber = 1,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_DEV_CLASS_COMMUNIC,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		// no interface string
		.iInterface = 0,
	},
	.ep4IN = {
		.bLength = sizeof(USB_ENDP_DESCR),
		.bDescriptorType = USB_DESCR_TYP_ENDP,
		// EP4 IN
		.bEndpointAddress = 0x84,
		.bmAttributes = USB_ENDP_TYPE_INTER,
		.wMaxPacketSizeL = HID_PKT_SIZ,
		.wMaxPacketSizeH = 0,
		// poll every 1 ms
		.bInterval = 1,
	},
	.ep4OUT = {
		.bLength = sizeof(USB_ENDP_DESCR),
		.bDescriptorType = USB_DESCR_TYP_ENDP,
		.bEndpointAddress = 0x03,       // EP1 IN
		.bmAttributes = USB_ENDP_TYPE_INTER,
		.wMaxPacketSizeL = HID_PKT_SIZ,
		.wMaxPacketSizeH = 0,
		.bInterval = 1,                 // poll every 1 ms
	},
	/* i2c */
	.i2c_interface = {
		.bLength = sizeof(USB_ITF_DESCR),
		.bDescriptorType = USB_DESCR_TYP_INTERF,
		.bInterfaceNumber = 2,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_DEV_CLASS_HID,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,                // no interface string
	},
	.ep1IN = {
		.bLength = sizeof(USB_ENDP_DESCR),
		.bDescriptorType = USB_DESCR_TYP_ENDP,
		.bEndpointAddress = 0x81,       // EP1 IN
		.bmAttributes = USB_ENDP_TYPE_INTER,
		.wMaxPacketSizeL = HID_PKT_SIZ,
		.wMaxPacketSizeH = 0,
		.bInterval = 1,                 // poll every 1 ms
	},
	.ep1OUT = {
		.bLength = sizeof(USB_ENDP_DESCR),
		.bDescriptorType = USB_DESCR_TYP_ENDP,
		.bEndpointAddress = 0x1,       // EP1 out
		.bmAttributes = USB_ENDP_TYPE_INTER,
		.wMaxPacketSizeL = HID_PKT_SIZ,
		.wMaxPacketSizeH = 0,
		.bInterval = 1,                 // poll every 1 ms
	},
};

__code uint8_t ReportDesc[] ={
    0x06, 0x00, 0xFF,   // Usage Page = 0xFF00 (Vendor Defined Page 1)
    // USB-IF HID tool says vendor usage not required, but Win7 needs it
    0x09, 0x01,         // Usage (Vendor Usage 1)
    0xA1, 0x01,         // Collection (Application)
    0x15, 0x00,         //  Logical minimum value 0
    0x25, 0xFF,         //  Logical maximum value 255
    0x75, 0x08,         //  Report Size: 8-bit field size
    0x95, HID_PKT_SIZ,  //  Report Count: Make 64 fields

    // Input Report
    0x09, 0x01,         //  Usage (Vendor Usage 1)
    0x81, 0x02,         //  Input (Data,Var,Abs,No Wrap,Linear)

    // Output Report
    0x09, 0x01,         //  Usage (Vendor Usage 1)
    0x91, 0x02,         //  Output (Data,Var,Abs,No Wrap,Linear)

    0xC0                // End Collection
};
__code uint8_t ReportDescLen = sizeof(ReportDesc);

//String Descriptors
//Language Descriptor
__code uint16_t LangDes[]={
	0x0300 + sizeof(LangDes),   // type and length
	0x0409                      // 0x0409 = US English language code
};

//Product String Descriptor
__code uint16_t Prod_Des[]={
	0x0300 + sizeof(Prod_Des),  // type and length

	// compound literals not supported in SDCC as of v4.0.3
	// (__code uint16_t[]) { u"CH55x CMSIS-DAP" }
	'b','e','e','t','o','o', 'l',
};

__code uint16_t Manuf_Des[]={
	0x0300 + sizeof(Manuf_Des), // type and length
	'd','g','p'
};

/* Serial string descriptor */
__code uint16_t Ser_Des[]={
	0x0300 + sizeof(Ser_Des),   // type and length
	'N','R',
};
