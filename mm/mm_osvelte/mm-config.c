// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2024 Oplus. All rights reserved.
 */
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "sys-memstat.h"
#include "mm-config.h"

struct config_data {
	const char *module_name;
	void *private;
	struct list_head list;
	void (*seq_show)(struct seq_file *m, struct config_data *cd);
};
unsigned long cmdline_mm_reserved_1;

static LIST_HEAD(config_list);

/******************************************************************************
 *                          ta_cma_rsv
 ******************************************************************************/

static int config_list_show(struct seq_file *m, void *data)
{
	/* module is initialized at boot stage, so no need lock to protect. */
	struct config_data *cd;
	seq_printf(m, "cmdline: %lx\n", cmdline_mm_reserved_1);

	list_for_each_entry(cd, &config_list, list)
		cd->seq_show(m, cd);
	return 0;
}
DEFINE_PROC_SHOW_ATTRIBUTE(config_list);

void *oplus_read_mm_config(const char *module_name)
{
	struct config_data *cd;

	if (!module_name)
		return NULL;

	list_for_each_entry(cd, &config_list, list) {
		if (strcmp(module_name, cd->module_name) == 0)
			return cd->private;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(oplus_read_mm_config);

bool oplus_test_mm_feature_disable(unsigned long nr)
{
	return (1ul << nr) & cmdline_mm_reserved_1;
}
EXPORT_SYMBOL_GPL(oplus_test_mm_feature_disable);

static const struct of_device_id mm_config_match_table[] = {
	{.compatible = "oplus,mm_osvelte-config", },
	{},
};
MODULE_DEVICE_TABLE(of, mm_config_match_table);
