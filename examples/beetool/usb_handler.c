#include <string.h>
#include <stdio.h>
#include <ch554.h>
#include <ch554_usb.h>

#include "config.h"
#include "usb_handler.h"
#include "usb_descriptor.h"

#define UsbSetupBuf     ((PUSB_SETUP_REQ)epbuffer_ep0)

//on page 47 of data sheet, the receive buffer need to be min(possible packet size+2,64)
__xdata uint8_t  epbuffer_ep0[DEFAULT_ENDP0_SIZE + 2];

#ifdef CONFIG_EP1_ENABLE
__xdata uint8_t  epbuffer_ep1[CONFIG_EP1_BUFFERSZ];
#endif

#ifdef CONFIG_EP2_ENABLE
__xdata uint8_t  epbuffer_ep2[EP2_BUFFER_SZ] = { 0 };
#endif

#ifdef CONFIG_EP3_ENABLE
__xdata uint8_t  epbuffer_ep3[CONFIG_EP3_BUFFERSZ];
#endif

#ifdef CONFIG_EP4_ENABLE
__xdata uint8_t  Ep4Buffer[CONFIG_EP4_BUFFERSZ];
#endif

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
	mov DPL, #_epbuffer_ep0
	mov DPH, #(_epbuffer_ep0 >> 8)
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

static void usb_ep0_setup(void)
{
	uint8_t len = USB_RX_LEN;
	if (len == (sizeof(USB_SETUP_REQ)))
			{
		uint16_t wLength = ((uint16_t)UsbSetupBuf->wLengthH << 8)
				| (UsbSetupBuf->wLengthL);
		SetupLen = wLength;
		// maximum supported reply size is 254 bytes
		if (wLength > 254)
			SetupLen = 254;
		len = 0;               // Default is success and upload 0 length
		SetupReq = UsbSetupBuf->bRequest;
		usbMsgFlags = 0;
		if (( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK)
				!= USB_REQ_TYP_STANDARD)  //Not standard request
		{
			switch (( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK))
			{
			case USB_REQ_TYP_VENDOR:
				{
				switch (SetupReq)
				{
				default:
					len = 0xFF; //command not supported
					break;
				}
				break;
			}
			case USB_REQ_TYP_CLASS:
				{
				switch (SetupReq)
				{
				default:
					len = 0xFF; //command not supported
					break;
				}
				break;
			}
			default:
				len = 0xFF;         //command not supported
				break;
			}

		}
		else                           //Standard request
		{
			switch (SetupReq)           //Request ccfType
			{
			case USB_GET_DESCRIPTOR:
				switch (UsbSetupBuf->wValueH)
				{
				case 1:             // Device Descriptor
					pDescr = (uint8_t*)&DevDesc;
					len = sizeof(DevDesc);
					break;
				case 2:             // configureation descriptor
					pDescr = CfgDesc;
					len = CfgDescLen;
					break;
				case 3:             // string descriptor
					if (UsbSetupBuf->wValueL == 0)
							{
						pDescr = LangDes;
					}
					else if (UsbSetupBuf->wValueL == 1)
							{
						pDescr = Manuf_Des;
					}
					else if (UsbSetupBuf->wValueL == 2)
							{
						pDescr = Prod_Des;
					}
					else
					{
						pDescr = Ser_Des;
					}
					len = pDescr[0];    // bLength
					break;
				case USB_DESCR_TYP_REPORT:
					if (UsbSetupBuf->wValueL == 0) {
						pDescr = ReportDesc;
						len = ReportDescLen;
					}
					else
					{
						len = 0xff;
					}
					break;
				default:
					len = 0xff; // Unsupported descriptors or error
					break;
				}
				if (len != 0xff) {
					if (SetupLen > len)
							{
						SetupLen = len;  // Limit length
					}
					len = SetupLen >= DEFAULT_ENDP0_SIZE ?
							DEFAULT_ENDP0_SIZE :
							SetupLen; //transmit length for this packet
					cpy_desc_Ep0(len);

					SetupLen -= len;
					pDescr += len;
				}
				break;
			case USB_SET_ADDRESS:
				SetupLen = UsbSetupBuf->wValueL; // Save the assigned address
				break;
			case USB_GET_CONFIGURATION:
				epbuffer_ep0[0] = UsbConfig;
				if (SetupLen >= 1)
						{
					len = 1;
				}
				break;
			case USB_SET_CONFIGURATION:
				UsbConfig = UsbSetupBuf->wValueL;
				break;
			case USB_GET_INTERFACE:
				break;
			case USB_SET_INTERFACE:
				break;
			case USB_CLEAR_FEATURE:
				if (( UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_DEVICE) // Clear the device featuee.
				{
					if ((((uint16_t)UsbSetupBuf->wValueH
							<< 8)
							| UsbSetupBuf->wValueL)
							== 0x01)
							{
						if (CfgDesc[7] & 0x20)
								{
							// wake up
						}
						else
						{
							len = 0xFF;     //Failed
						}
					}
					else
					{
						len = 0xFF;             //Failed
					}
				}
				else if (( UsbSetupBuf->bRequestType
						& USB_REQ_RECIP_MASK)
						== USB_REQ_RECIP_ENDP) // endpoint
				{
					switch ( UsbSetupBuf->wIndexL)
					{
					case 0x84:
						UEP4_CTRL =
								UEP4_CTRL
										& ~( bUEP_T_TOG
												| MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x04:
						UEP4_CTRL =
								UEP4_CTRL
										& ~( bUEP_R_TOG
												| MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					case 0x83:
						UEP3_CTRL =
								UEP3_CTRL
										& ~( bUEP_T_TOG
												| MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x03:
						UEP3_CTRL =
								UEP3_CTRL
										& ~( bUEP_R_TOG
												| MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					case 0x82:
						UEP2_CTRL =
								UEP2_CTRL
										& ~( bUEP_T_TOG
												| MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x02:
						UEP2_CTRL =
								UEP2_CTRL
										& ~( bUEP_R_TOG
												| MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					case 0x81:
						UEP1_CTRL =
								UEP1_CTRL
										& ~( bUEP_T_TOG
												| MASK_UEP_T_RES)| UEP_T_RES_NAK;
						break;
					case 0x01:
						UEP1_CTRL =
								UEP1_CTRL
										& ~( bUEP_R_TOG
												| MASK_UEP_R_RES)| UEP_R_RES_ACK;
						break;
					default:
						len = 0xFF; // Unsupported endpoint
						break;
					}
				}
				else
				{
					len = 0xFF; // Unsupported for non-endpoint
				}
				break;
			case USB_SET_FEATURE:                     // Set Feature
				if (( UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_DEVICE) // Set  the device featuee.
				{
					if ((((uint16_t)UsbSetupBuf->wValueH
							<< 8)
							| UsbSetupBuf->wValueL)
							== 0x01)
							{
						if (CfgDesc[7] & 0x20)
								{
							// suspend not supported
						}
						else
						{
							len = 0xFF;    // Failed
						}
					}
					else
					{
						len = 0xFF;            // Failed
					}
				}
				else if (( UsbSetupBuf->bRequestType & 0x1F)
						== USB_REQ_RECIP_ENDP) //endpoint
				{
					if ((((uint16_t)UsbSetupBuf->wValueH
							<< 8)
							| UsbSetupBuf->wValueL)
							== 0x00)
							{
						switch (((uint16_t)UsbSetupBuf->wIndexH
								<< 8)
								| UsbSetupBuf->wIndexL)
						{
						case 0x84:
							UEP4_CTRL =
									UEP4_CTRL
											& (~bUEP_T_TOG)| UEP_T_RES_STALL; // Set endpoint4 IN STALL
							break;
						case 0x04:
							UEP4_CTRL =
									UEP4_CTRL
											& (~bUEP_R_TOG)| UEP_R_RES_STALL; // Set endpoint4 OUT Stall
							break;
						case 0x83:
							UEP3_CTRL =
									UEP3_CTRL
											& (~bUEP_T_TOG)| UEP_T_RES_STALL; // Set endpoint3 IN STALL
							break;
						case 0x03:
							UEP3_CTRL =
									UEP3_CTRL
											& (~bUEP_R_TOG)| UEP_R_RES_STALL; // Set endpoint3 OUT Stall
							break;
						case 0x82:
							UEP2_CTRL =
									UEP2_CTRL
											& (~bUEP_T_TOG)| UEP_T_RES_STALL; // Set endpoint2 IN STALL
							break;
						case 0x02:
							UEP2_CTRL =
									UEP2_CTRL
											& (~bUEP_R_TOG)| UEP_R_RES_STALL; // Set endpoint2 OUT Stall
							break;
						case 0x81:
							UEP1_CTRL =
									UEP1_CTRL
											& (~bUEP_T_TOG)| UEP_T_RES_STALL; // Set endpoint1 IN STALL
							break;
						case 0x01:
							UEP1_CTRL =
									UEP1_CTRL
											& (~bUEP_R_TOG)| UEP_R_RES_STALL; // Set endpoint1 OUT Stall
						default:
							len = 0xFF;    // Failed
							break;
						}
					}
					else
					{
						len = 0xFF;            // Failed
					}
				}
				else
				{
					len = 0xFF;                    // Failed
				}
				break;
			case USB_GET_STATUS:
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
				break;
			default:
				len = 0xff;                            // Failed
				break;
			}
		}
	}
	else
	{
		len = 0xff;                                //Wrong packet length
	}
	if (len == 0xff)
			{
		SetupReq = 0xFF;
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL
				| UEP_T_RES_STALL;                       //STALL
	}
	else if (len <= DEFAULT_ENDP0_SIZE) // Tx data to host or send 0-length packet
	{
		UEP0_T_LEN = len;
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK
				| UEP_T_RES_ACK;      //Expect DATA1, Answer ACK
	}
	else
	{
		// TODO: remove unreachable code here
		UEP0_T_LEN = 0;  // Tx data to host or send 0-length packet
		UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK
				| UEP_T_RES_ACK;  //Expect DATA1, Answer ACK
	}
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
{							\
	usb_stats.ENDPOINT_STAT(which, direction)++;	\
	usb_ep##which##_##direction();			\
}

#define BADENDPOINT() usb_stats.bad_ep++;

static inline int usb_interrupt_fifo_ov(void)
{
	if (!UIF_FIFO_OV)
		return 0;

	usb_stats.ovr++;
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
