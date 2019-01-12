/*
 * Copyright (c) 2018 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <nrf_power.h>

static int board_badge_init(struct device *dev)
{
	ARG_UNUSED(dev);

	return 0;
}

SYS_INIT(board_badge_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
