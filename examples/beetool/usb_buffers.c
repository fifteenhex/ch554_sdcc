/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include "usb_buffers.h"

/*
 * on page 47 of data sheet, the receive buffer
 * need to be min(possible packet size+2,64)
 */
__xdata union ep0_composite epbuffer_ep0_composite;

#ifdef CONFIG_EP1_ENABLE
__xdata uint8_t epbuffer_ep1[EP1_BUFFER_SZ];
#endif

#ifdef CONFIG_EP2_ENABLE
__xdata uint8_t epbuffer_ep2[EP2_BUFFER_SZ];
#endif

#ifdef CONFIG_EP3_ENABLE
__xdata uint8_t epbuffer_ep3[EP3_BUFFER_SZ];
#endif

#ifdef CONFIG_EP4_ENABLE
__xdata uint8_t epbuffer_ep4[EP4_BUFFER_SZ];
#endif
