/* SPDX-License-Identifier: GPL-3.0 */
/* Copyright 2021 - Daniel Palmer <daniel@thingy.jp> */

#ifndef _CDC_H_
#define _CDC_H_

#include <stdint.h>

#include "config.h"

struct cdc_stats {
	uint8_t send_encapsulated_command;
	uint8_t get_encapsulated_response;
};

extern __xdata struct cdc_stats cdc_stats;

#ifdef CONFIG_CDC_ACM
int cdc_setup_class(void);
void cdc_main_loop(void);
#else
#define cdc_setup_class() do { } while (0)
#define cdc_main_loop() do { } while (0)
#endif

#endif
