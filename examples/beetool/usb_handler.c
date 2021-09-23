#include <string.h>
#include <stdio.h>
#include <ch554.h>
#include <ch554_usb.h>

#include "config.h"
#include "cdc.h"
#include "usb_buffers.h"
#include "usb_handler.h"
#include "usb_descriptor.h"

#ifdef CONFIG_USB_PKTDBG
__xdata struct usb_stats usb_stats = { 0 };

#define usb_stat_inc(which)		\
	do {				\
		usb_stats.which++;	\
	} while(0)
#else
#define usb_stat_inc(which) do { } while(0)
#endif


static uint8_t SetupLen;
static uint8_t SetupReq,UsbConfig;

__code uint8_t *pDescr;

volatile uint8_t usbMsgFlags=0;    // uint8_t usbMsgFlags copied from VUSB

inline void NOP_Process(void) {}

// copy descriptor *pDescr to Ep0
#pragma callee_saves cpy_desc_Ep0
static void cpy_desc_Ep0(uint8_t len) __naked
{
	// stop unused arg warning
	len;
	__asm
	xch A, _DPL     ; ACC = len
	inc _XBUS_AUX
	mov DPL, #_epbuffer_ep0_composite
	mov DPH, #(_epbuffer_ep0_composite >> 8)
	dec _XBUS_AUX
	mov DPL, _pDescr
	mov DPH, (_pDescr + 1)
	sjmp _ccpyx
	__endasm;
}

// copy code to xram 
// *dest in DPTR1, len in A
#pragma callee_saves ccpyx
static void ccpyx(__code char* src)
{
	// stop unused arg warning
	src;
	__asm
	push ar7
	xch A, R7
	01$:
	clr A
	movc A, @A+DPTR
	inc DPTR
	.DB  0xA5       ;MOVX @DPTR1,A & INC DPTR1
	djnz R7, 01$
	pop ar7
	__endasm;
}

static inline void usb_ep0_setup_send_response(size_t len)
{
	UEP0_T_LEN = len;
	UEP0_CTRL = bUEP_R_TOG |
		    bUEP_T_TOG |
		    UEP_R_RES_ACK |
		    UEP_T_RES_ACK;
}

static inline int usb_ep0_setup_vendor(void)
{
	switch (SetupReq)
	{
	default:
		return 1;
	}

	return 0;
}

/*
 * Class setup request handlers should be
 * added here.
 */
static inline int usb_ep0_setup_class()
{
	return cdc_setup_class();
}

static inline int usb_ep0_setup_get_descriptor(void)
{
	int len;

	switch (setupreq.wValueH)
	{
	/* Device Descriptor */
	case 1:
		pDescr = (uint8_t*)&DevDesc;
		len = sizeof(DevDesc);
		break;
	/* Configuration descriptor */
	case 2:
		pDescr = CfgDesc;
		len = CfgDescLen;
		break;
	/* String descriptor */
	case 3:
		if (setupreq.wValueL == 0)
			pDescr = LangDes;
		else if (setupreq.wValueL == 1)
			pDescr = Manuf_Des;
		else if (setupreq.wValueL == 2)
			pDescr = Prod_Des;
		else
			pDescr = Ser_Des;
		// bLength
		len = pDescr[0];
		break;
	case USB_DESCR_TYP_REPORT:
		if (setupreq.wValueL == 0) {
			pDescr = ReportDesc;
			len = ReportDescLen;
		}
		else
			return 1;
		break;
	default:
		// Unsupported descriptors or error
		return 1;
	}

	if (SetupLen > len)
		SetupLen = len;  // Limit length

	//transmit length for this packet
	len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE :
			SetupLen;
	cpy_desc_Ep0(len);

	SetupLen -= len;
	pDescr += len;

	usb_ep0_setup_send_response(len);

	return 0;
}

static inline int usb_ep0_setup_clear_feature(void)
{
	// Clear the device feature.
	if (( setupreq.bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE) {
		if ((((uint16_t)setupreq.wValueH << 8) | setupreq.wValueL) == 0x01) {
			if (CfgDesc[7] & 0x20){
				// wake up
			}
			else
				return 1;
		}
		else
			return 1;
	}
	// endpoint
	else if (( setupreq.bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
	{
		switch ( setupreq.wIndexL)
		{
#ifdef CONFIG_EP1_ENABLE
		case 0x81:
			UEP1_CTRL = UEP1_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
			break;
		case 0x01:
			UEP1_CTRL = UEP1_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
			break;
#endif
#ifdef CONFIG_EP2_ENABLE
		case 0x82:
			UEP2_CTRL = UEP2_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
			break;
		case 0x02:
			UEP2_CTRL = UEP2_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
			break;
#endif
#ifdef CONFIG_EP3_ENABLE
		case 0x83:
			UEP3_CTRL = UEP3_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
			break;
		case 0x03:
			UEP3_CTRL = UEP3_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
			break;
#endif
#ifdef CONFIG_EP4_ENABLE
		case 0x84:
			UEP4_CTRL = UEP4_CTRL & ~( bUEP_T_TOG | MASK_UEP_T_RES)| UEP_T_RES_NAK;
			break;
		case 0x04:
			UEP4_CTRL = UEP4_CTRL & ~( bUEP_R_TOG | MASK_UEP_R_RES)| UEP_R_RES_ACK;
			break;
#endif
		default:
			return 1;
		}
	}
	else
		return 1;

	usb_ep0_setup_send_response(0);

	return 0;
}

static inline int usb_ep0_setup_set_feature(void)
{
	/* Set  the device feature. */
	if (( setupreq.bRequestType & 0x1F) == USB_REQ_RECIP_DEVICE)
	{
		if ((((uint16_t)setupreq.wValueH << 8) | setupreq.wValueL) == 0x01) {
			if (CfgDesc[7] & 0x20)
			{
				// suspend not supported
			}
			else
				return 1;
		}
		else
			return 1;
	}
	//endpoint
	else if ((setupreq.bRequestType & 0x1F) == USB_REQ_RECIP_ENDP) {
		if ((((uint16_t)setupreq.wValueH << 8) | setupreq.wValueL) == 0x00)
				{
			switch (((uint16_t)setupreq.wIndexH << 8) | setupreq.wIndexL)
			{
#ifdef CONFIG_EP1_ENABLE
			case 0x81:
				// Set endpoint1 IN STALL
				UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG)| UEP_T_RES_STALL;
				break;
			case 0x01:
				// Set endpoint1 OUT Stall
				UEP1_CTRL = UEP1_CTRL & (~bUEP_R_TOG)| UEP_R_RES_STALL;
#endif
#ifdef CONFIG_EP2_ENABLE
			case 0x82:
				// Set endpoint2 IN STALL
				UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG)| UEP_T_RES_STALL;
				break;
			case 0x02:
				// Set endpoint2 OUT Stall
				UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG)| UEP_R_RES_STALL;
				break;
#endif
#ifdef CONFIG_EP3_ENABLE
			case 0x83:
				// Set endpoint3 IN STALL
				UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG)| UEP_T_RES_STALL;
				break;
			case 0x03:
				// Set endpoint3 OUT Stall
				UEP3_CTRL = UEP3_CTRL & (~bUEP_R_TOG)| UEP_R_RES_STALL;
				break;
#endif
#ifdef CONFIG_EP4_ENABLE
			case 0x84:
				// Set endpoint4 IN STALL
				UEP4_CTRL = UEP4_CTRL & (~bUEP_T_TOG)| UEP_T_RES_STALL;
				break;
			case 0x04:
				// Set endpoint4 OUT Stall
				UEP4_CTRL = UEP4_CTRL & (~bUEP_R_TOG)| UEP_R_RES_STALL;
				break;
#endif
			default:
				return 1;
			}
		}
		else
			return 1;
	}
	else
		return 1;

	usb_ep0_setup_send_response(0);

	return 0;
}

static inline int usb_ep0_setup_get_status(void)
{
	int len;

	epbuffer_ep0[0] = 0x00;
	epbuffer_ep0[1] = 0x00;
	if (SetupLen >= 2)
			{
		len = 2;
	}
	else
	{
		len = SetupLen;
	}

	usb_ep0_setup_send_response(len);

	return 0;

}

static inline int usb_ep0_setup_set_address(void)
{
	// Save the assigned address
	SetupLen = setupreq.wValueL;

	usb_ep0_setup_send_response(0);

	return 0;
}

static inline int usb_ep0_setup_get_configuration(void)
{
	int len;

	epbuffer_ep0[0] = UsbConfig;
	if (SetupLen >= 1)
			{
		len = 1;
	}

	usb_ep0_setup_send_response(len);

	return 0;
}

static inline int usb_ep0_setup_set_configuration(void)
{
	UsbConfig = setupreq.wValueL;
	usb_ep0_setup_send_response(0);

	return 0;
}

static inline int usb_ep0_setup_standard(void)
{
	switch (SetupReq)           //Request ccfType
	{
	case USB_GET_DESCRIPTOR:
		return usb_ep0_setup_get_descriptor();
	case USB_SET_ADDRESS:
		return usb_ep0_setup_set_address();
	case USB_GET_CONFIGURATION:
		return usb_ep0_setup_get_configuration();
	case USB_SET_CONFIGURATION:
		return usb_ep0_setup_set_configuration();
	case USB_GET_INTERFACE:
		break;
	case USB_SET_INTERFACE:
		break;
	case USB_CLEAR_FEATURE:
		return usb_ep0_setup_clear_feature();
	case USB_SET_FEATURE:
		return usb_ep0_setup_set_feature();
	case USB_GET_STATUS:
		return usb_ep0_setup_get_status();
	}
	return 1;
}

static inline int usb_ep0_setup_nonstandard(void)
{
	switch (( setupreq.bRequestType & USB_REQ_TYP_MASK))
	{
	case USB_REQ_TYP_VENDOR:
		return usb_ep0_setup_vendor();
	case USB_REQ_TYP_CLASS:
		return usb_ep0_setup_class();
	default:
		return 1;
	}
}

static inline void usb_ep0_setup(void)
{
	int ret;

	uint8_t len = USB_RX_LEN;

	if (len != (sizeof(USB_SETUP_REQ)))
		goto unhandled;

	uint16_t wLength = ((uint16_t)setupreq.wLengthH << 8) |
			(setupreq.wLengthL);

	SetupLen = wLength;
	// maximum supported reply size is 254 bytes
	if (wLength > 254)
		SetupLen = 254;
	len = 0;               // Default is success and upload 0 length
	SetupReq = setupreq.bRequest;
	usbMsgFlags = 0;

	if (( setupreq.bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
		ret = usb_ep0_setup_standard();
	else
		ret = usb_ep0_setup_nonstandard();

	if(!ret)
		return;

unhandled:
	usb_stat_inc(setup_unhandled);
	SetupReq = 0xFF;
	//STALL
	UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL
			| UEP_T_RES_STALL;
}

static void usb_ep0_in(void)
{
	switch (SetupReq) {
	case USB_GET_DESCRIPTOR: {
		//send length
		uint8_t len = SetupLen >= DEFAULT_ENDP0_SIZE ? DEFAULT_ENDP0_SIZE : SetupLen;
		cpy_desc_Ep0(len);
		SetupLen -= len;
		pDescr += len;
		UEP0_T_LEN = len;
		//Switch between DATA0 and DATA1
		UEP0_CTRL ^= bUEP_T_TOG;
	}
	break;
	case USB_SET_ADDRESS:
		USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
		UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
	break;
	default:
		UEP0_T_LEN = 0;
		// End of transaction
		UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
	break;
	}
}

static void usb_ep0_out(void)
{
	UEP0_T_LEN = 0;
	//Respond Nak
	UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_NAK;
}

#define ENDPOINT_STAT(which, direction) direction##_ep##which

#define CALLENDPOINT(which, direction)			\
do {							\
	usb_stat_inc(ENDPOINT_STAT(which, direction));	\
	usb_ep##which##_##direction();			\
} while(0)

#define BADENDPOINT()		\
do {				\
	usb_stat_inc(bad_ep);	\
} while(0)

static inline int usb_interrupt_fifo_ov(void)
{
	if (!UIF_FIFO_OV)
		return 0;

	usb_stat_inc(ovr);
	return 1;
}

static inline void usb_interrupt_tx_out(uint8_t callIndex)
{
	//SDCC will take IRAM if array of function pointer is used.
	switch (callIndex) {
	case 0:
		CALLENDPOINT(0, out);
		break;
	case 1:
#ifdef CONFIG_EP1_OUT
	CALLENDPOINT(1, out);
#else
	BADENDPOINT();
#endif
		break;
	case 2:
#ifdef CONFIG_EP2_OUT
	CALLENDPOINT(2, out);
#else
	BADENDPOINT();
#endif
		break;
	case 3:
#ifdef CONFIG_EP3_OUT
	CALLENDPOINT(3, out);
#else
	BADENDPOINT();
#endif
		break;
	case 4:
		break;
	default:
		break;
	}
}

static inline void usb_interrupt_tx_in(uint8_t callIndex)
{
	//SDCC will take IRAM if array of function pointer is used.
	switch (callIndex) {
	case 0:
		CALLENDPOINT(0, in);
		break;
	case 1:
#ifdef CONFIG_EP1_IN
		CALLENDPOINT(1, in);
#else
		BADENDPOINT();
#endif
		break;
	case 2:
#ifdef CONFIG_EP2_IN
		CALLENDPOINT(2, in);
#else
		BADENDPOINT();
#endif
		break;
	case 3:
#ifdef CONFIG_EP3_IN
		CALLENDPOINT(3, in);
#else
		BADENDPOINT();
#endif
		break;
	case 4:
		break;
	default:
		break;
	}
}

static inline void usb_interrupt_tx_setup(uint8_t callIndex)
{
	usb_stat_inc(setup);

	switch (callIndex) {
	case 0:
		usb_ep0_setup();
		break;
	default:
		break;
	}
}

static inline int usb_interrupt_tx(void)
{
	if (!UIF_TRANSFER)
		return 0;

	usb_stat_inc(tx);

	/* Discard unsynchronized packets */
	//if (!U_TOG_OK) {
	//	usb_stats.tog_ng++;
	//	return 1;
	//}
	// Dispatch to service functions
	uint8_t callIndex = USB_INT_ST & MASK_UIS_ENDP;
	switch (USB_INT_ST & MASK_UIS_TOKEN) {
	case UIS_TOKEN_OUT:
		usb_interrupt_tx_out(callIndex);
		break;
	case UIS_TOKEN_IN:
		usb_interrupt_tx_in(callIndex);
		break;
	case UIS_TOKEN_SETUP:
		usb_interrupt_tx_setup(callIndex);
		break;
	default:
		usb_stat_inc(unhandled);
		break;
	}

	/* Clear interrupt flag */
	UIF_TRANSFER = 0;

	return 1;
}

static inline int usb_interrupt_rst(void)
{
	// Device mode USB bus reset
	if (!UIF_BUS_RST)
		return 0;

	usb_stat_inc(rst);

	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
	//Endpoint 1 automatically flips the sync flag, and IN transaction returns NAK
	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
	//Endpoint 2 automatically flips the sync flag, IN transaction returns NAK, OUT returns ACK
	UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
	//UEP4_CTRL = UEP_T_RES_NAK | UEP_R_RES_ACK;  //bUEP_AUTO_TOG only work for endpoint 1,2,3

	USB_DEV_AD = 0;
	UIF_SUSPEND = 0;
	UIF_TRANSFER = 0;
	// Clear interrupt flag
	UIF_BUS_RST = 0;

	return 1;
}

static inline int usb_interrupt_sus(void)
{
	// USB bus suspend / wake up
	if (!UIF_SUSPEND)
		return 0;

	usb_stat_inc(sus);

	UIF_SUSPEND = 0;
	if (USB_MIS_ST & bUMS_SUSPEND) {
		// suspend not supported
	} else {    // Unexpected interrupt, not supposed to happen !
		USB_INT_FG = 0xFF;        // Clear interrupt flag
	}

	return 1;
}

#pragma save
#pragma nooverlay
//inline not really working in multiple files in SDCC
void usb_interrupt(void)
{
	uint8_t handled = 0;
	usb_stat_inc(irqs);

	handled |= usb_interrupt_fifo_ov();
	handled |= usb_interrupt_tx();
	handled |= usb_interrupt_rst();
	handled |= usb_interrupt_sus();

	if(!handled)
		usb_stat_inc(spurious);
}
#pragma restore

void usb_configure()
{
	//USB internal pull-up enable, return NAK if USB INT flag not clear
	USB_CTRL = bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;

	// enable port, full-speed, disable UDP/UDM pulldown resistor
	UDEV_CTRL = bUD_PD_DIS | bUD_PORT_EN;

	// configure interrupts
	// USB_INT_EN |= bUIE_SUSPEND;         //Enable device hang interrupt
	USB_INT_EN |= bUIE_TRANSFER;        //Enable USB transfer completion interrupt
	USB_INT_EN |= bUIE_BUS_RST;         //Enable device mode USB bus reset interrupt
	USB_INT_FG |= 0x1F;                 //Clear interrupt flag
	IE_USB = 1;                         //Enable USB interrupt
	EA = 1;                             //Enable global interrupts

	/* configure endpoints */
	UEP0_DMA = (uint16_t) epbuffer_ep0;    //Endpoint 0 data transfer address
	UEP0_T_LEN = 0;
	//Manual flip, OUT transaction returns ACK, IN transaction returns NAK
	UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;

#ifdef CONFIG_EP1_ENABLE
	/* Endpoint data transfer address */
	UEP1_DMA = (uint16_t) epbuffer_ep1;
	/* Endpoint 1 automatically flips the sync flag, IN transaction returns NAK,
	 * OUT returns ACK
	 */
	UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
	UEP1_T_LEN = 0;
#endif

#ifdef CONFIG_EP2_ENABLE
	/* Endpoint data transfer address */
	UEP2_DMA = (uint16_t) epbuffer_ep2;

	UEP2_CTRL =	UEP_T_RES_NAK | UEP_R_RES_NAK;
	UEP2_T_LEN = 0;

#ifdef CONFIG_EP2_OUT
	UEP2_3_MOD |= bUEP2_RX_EN;
#endif

#endif

#ifdef CONFIG_EP3_ENABLE
	/* Endpoint data transfer address */
	UEP3_DMA = (uint16_t) epbuffer_ep3;
	//Endpoint 2 automatically flips the sync flag, IN & OUT transaction returns NAK
	UEP3_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;
	UEP3_T_LEN = 0;
#endif

#ifdef CONFIG_EP4_ENABLE
	/* Endpoint data transfer address */
	UEP4_DMA = (uint16_t) Ep4Buffer;
	//Endpoint 2 automatically flips the sync flag, IN & OUT transaction returns NAK
	UEP4_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_NAK;
	UEP4_T_LEN = 0;
#endif

	UEP4_1_MOD = bUEP1_RX_EN;
}

#ifdef CONFIG_USB_PKTDBG
void usb_printstats()
{
	static uint16_t lastirqs = 0;

	/*
	 * If the irq happens just after reading the
	 * counter this will skip printing out until
	 * being called again, this is a debug function
	 * so lets not worry too much about that.
	 */
	if(usb_stats.irqs == lastirqs)
			return;
	lastirqs = usb_stats.irqs;

	printf("irqs: %u\r\n", usb_stats.irqs);
	printf("\tovr: %u\r\n", usb_stats.ovr);

	printf("\ttx: %u\r\n", usb_stats.tx);
	printf("\t\tbad ep: %u\r\n", usb_stats.bad_ep);
	printf("\t\ttog ng: %u\r\n", usb_stats.tog_ng);
	printf("\t\tunhandled: %u\r\n", usb_stats.unhandled);

	printf("\t\tsetup: %u\r\n", usb_stats.setup);
	printf("\t\t\tunhandled: %u\r\n", usb_stats.setup_unhandled);

	printf("\trst: %u\r\n", usb_stats.rst);
	printf("\tsus: %u\r\n", usb_stats.sus);
	printf("\tspurious: %u\r\n", usb_stats.spurious);
	printf("EP0: in %u\r\n", usb_stats.in_ep0);
	printf("EP0: out %u\r\n", usb_stats.out_ep0);

#ifdef CONFIG_EP1_IN
	printf("EP1: in %u\r\n", usb_stats.in_ep1);
#endif
#ifdef CONFIG_EP1_OUT
	printf("EP1: out %u\r\n", usb_stats.out_ep1);
#endif

#ifdef CONFIG_EP2_IN
	printf("EP2: in %u\r\n", usb_stats.in_ep2);
#endif
#ifdef CONFIG_EP2_OUT
	printf("EP2: out %u\r\n", usb_stats.out_ep2);
#endif

#ifdef CONFIG_EP3_IN
	printf("EP3: in %u\r\n", usb_stats.in_ep3);
#endif
#ifdef CONFIG_EP3_OUT
	printf("EP3: out %u\r\n", usb_stats.out_ep3);
#endif
}
#else
void usb_printstats()
{
}
#endif
