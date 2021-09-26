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
	.idVendor = 0x16d0,
	.idProduct = 0x10d3,
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
	/* i2c - to avoid having to make big driver changes this comes first */
	USB_ITF_DESCR i2c_interface;
	/* cdc */
	struct usb_interface_association_descriptor cdc_iad;
	USB_ITF_DESCR cdc_control_interface;
	USB_HEADERFUNCTIONAL_DESCR cdc_header_functional;
	USB_ACMFUNCTIONAL_DESCR cdc_acm_functional;
	USB_UNIONFUNCTIONAL_DESCR cdc_union_functional;
	USB_CALLMANAGEMENTFUNCTIONAL_DESCR cdc_call_management_functional;
	USB_ENDP_DESCR cdc_ctrl_ep;
	USB_ITF_DESCR cdc_data_interface;
	USB_ENDP_DESCR cdc_data_in_ep;
	USB_ENDP_DESCR cdc_data_out_ep;

} CfgDesc = {
	.config = CFG_DESCR(3),
	/* i2c */
	.i2c_interface = INTERF_DESCR(0, 0, USB_DEV_CLASS_VEN_SPEC, 0),
	/* cdc iad */
	.cdc_iad = usb_descriptor_interface_association_cdc_acm(1),
	/* cdc control */
	.cdc_control_interface = INTERF_DESCR(1, 1,USB_DEV_CLASS_COMMUNIC, USB_DEV_SUBCLASS_CDC_ACM),
	.cdc_header_functional = HEADERFUNCTIONAL_DESCR(),
	.cdc_acm_functional = ACMFUNCTIONAL_DESCR(0xf),
	.cdc_union_functional = UNIONFUNCTIONAL_DESCR(1, 2),
	.cdc_call_management_functional = CALLMANAGEMENTFUNCTIONAL_DESCR(2),
	.cdc_ctrl_ep = ENDPOINT_DESCR(1, 1, USB_ENDP_TYPE_INTER),
	/* cdc data */
	.cdc_data_interface = INTERF_DESCR(2, 2, USB_DEV_CLASS_CDC_DATA, 0),
	.cdc_data_in_ep = ENDPOINT_DESCR(2, 1, USB_ENDP_TYPE_BULK),
	.cdc_data_out_ep = ENDPOINT_DESCR(2, 0, USB_ENDP_TYPE_BULK),
};

/* String Descriptors */

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
