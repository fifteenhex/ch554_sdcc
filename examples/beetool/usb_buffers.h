/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef USB_BUFFERS_H_
#define USB_BUFFERS_H_

#include <stdint.h>
#include "config.h"

#define USB_BUFFERS_ENDPOINT_SZ_UNIT	64

extern __xdata uint8_t epbuffer_ep0[USB_BUFFERS_ENDPOINT_SZ_UNIT + 2];

/* endpoint 1 */
#ifdef CONFIG_EP1_ENABLE
#if defined(CONFIG_EP1_IN) && defined(CONFIG_EP1_OUT)
#define EP1_BUFFER_SZ 128
#else
#define EP1_BUFFER_SZ 64
#endif

extern __xdata uint8_t  epbuffer_ep1[EP1_BUFFER_SZ];
#endif

/* endpoint 2 */
#ifdef CONFIG_EP2_ENABLE
#if defined(CONFIG_EP2_IN) && defined(CONFIG_EP2_OUT)
#define EP2_BUFFER_SZ 128
#else
#define EP2_BUFFER_SZ 64
#endif

extern __xdata uint8_t epbuffer_ep2[EP2_BUFFER_SZ];
#endif

/* endpoint 3 */
#ifdef CONFIG_EP3_ENABLE
#if defined(CONFIG_EP3_IN) && defined(CONFIG_EP3_OUT)
#define EP3_BUFFER_SZ 128
#else
#define EP3_BUFFER_SZ 64
#endif

extern __xdata uint8_t epbuffer_ep3[EP3_BUFFER_SZ];
#endif

/* endpoint 4 */
#ifdef CONFIG_EP4_ENABLE
#if defined(CONFIG_EP4_IN) && defined(CONFIG_EP4_OUT)
#define EP4_BUFFER_SZ 128
#else
#define EP4_BUFFER_SZ 64
#endif

extern __xdata uint8_t epbuffer_ep4[EP4_BUFFER_SZ];
#endif

#endif
