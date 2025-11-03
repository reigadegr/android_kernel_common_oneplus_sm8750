// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#include <linux/err.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/ktime.h>
#include <asm/current.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#include "fpga_exception.h"

int fpga_exception_report(fpga_excep_type excep_tpye)
{
	return 0;
}
EXPORT_SYMBOL(fpga_exception_report);
