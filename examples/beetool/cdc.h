/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef _CDC_H_
#define _CDC_H_

#include <stdint.h>

#include "config.h"

#ifdef CONFIG_CDC_ACM_DEBUG
struct cdc_stats {
	uint8_t rx;
	uint8_t tx;
	uint8_t setlinecoding;
};

extern __xdata struct cdc_stats cdc_stats;
#endif

#ifdef CONFIG_CDC_ACM
int cdc_setup_class_irq(void);
int cdc_control_out_irq(void);
void cdc_main_loop(void);
#else
#define cdc_setup_class_irq() do { } while (0)
#define cdc_main_loop() do { } while (0)
#define cdc_control_out_irq do { } while (0)
#endif

/* endpoint irq callbacks */
void cdc_notification_in_irq(void);
void cdc_data_in_irq(void);
void cdc_data_out_irq(void);

#ifdef CONFIG_CDC_ACM_NOTIFICATION_EP1
#define usb_ep1_in_irq cdc_notification_in_irq
#endif

#ifdef CONFIG_CDC_ACM_DATA_EP2
#define usb_ep2_in_irq cdc_data_in_irq
#define usb_ep2_out_irq cdc_data_out_irq
#endif

#endif
