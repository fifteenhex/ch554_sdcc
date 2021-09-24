/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#include "usb_buffers.h"

__xdata union ep0_composite epbuffer_ep0_composite;

#ifdef CONFIG_EP1_ENABLE
__xdata struct ep1_composite epbuffer_ep1_composite;
#endif

#ifdef CONFIG_EP2_ENABLE
__xdata struct ep2_composite epbuffer_ep2_composite;
#endif

#ifdef CONFIG_EP3_ENABLE
__xdata uint8_t epbuffer_ep3[EP3_BUFFER_SZ];
#endif

#ifdef CONFIG_EP4_ENABLE
__xdata uint8_t epbuffer_ep4[EP4_BUFFER_SZ];
#endif
