
#define HID_PKT_SIZ 64

//#define CONFIG_USB_PKTDBG


#define CONFIG_EP1_ENABLE
#define CONFIG_EP1_IN
//#define CONFIG_EP1_OUT
#define CONFIG_EP1_BUFFERSZ	64

#define CONFIG_EP2_ENABLE
#define CONFIG_EP2_IN
#define CONFIG_EP2_OUT
#define CONFIG_EP2_BUFFERSZ	64

#define CONFIG_EP3_ENABLE
//#define CONFIG_EP3_IN
//#define CONFIG_EP3_OUT
#define CONFIG_EP3_BUFFERSZ	64

//#define CONFIG_EP4_ENABLE

#define CONFIG_CDC_DEBUG

#include <8052.h>
#define LED  P1_4
