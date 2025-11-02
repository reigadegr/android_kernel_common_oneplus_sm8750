// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2021 Oplus. All rights reserved.
 */
#include <linux/seq_file.h>
#include <linux/mm.h>

#include <trace/hooks/mm.h>

#include "common.h"

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

struct common_data {
	DECLARE_BITMAP(scene, BITS_PER_LONG);
	/* Prevent concurrent execution of device init */
	struct rw_semaphore init_lock;
	/* debug symbols */
	void *symbols[OMS_END];
	/* timer */
	bool timer_init;
	/* kobj */
	struct kobject *common_kobj;
	bool kill_bad_proc_enabled;
	unsigned int kill_bad_proc_pid;
	unsigned int kill_bad_proc_anon_kb;
	kallsyms_lookup_name_t kp_kallsyms_lookup_name;
};

/* replace this with pointer */
static struct common_data g_common;
bool osvelte_test_scene(unsigned long nr)
{
	struct common_data *data = &g_common;

	return test_bit(nr, data->scene);
}
EXPORT_SYMBOL_GPL(osvelte_test_scene);

void osvelte_register_symbol(enum oplus_mm_symbol sym, void *addr)
{
	struct common_data *data = &g_common;

	down_write(&data->init_lock);
	data->symbols[sym] = addr;
	up_write(&data->init_lock);
}
EXPORT_SYMBOL_GPL(osvelte_register_symbol);

void *osvelte_read_symbol(enum oplus_mm_symbol sym, bool atomic)
{
	struct common_data *data = &g_common;
	void *addr = NULL;

	if (sym >= OMS_END)
		goto out;

	/* for vendor hook, fast return, this may not safe */
	if (atomic)
		return data->symbols[sym];

	down_read(&data->init_lock);
	addr = data->symbols[sym];
	up_read(&data->init_lock);
out:
	return addr;
}
EXPORT_SYMBOL_GPL(osvelte_read_symbol);

void *osvelte_kallsyms_lookup_name(const char *name)
{
	struct common_data *data = &g_common;

	if (unlikely(!data->kp_kallsyms_lookup_name))
		return NULL;

	return (void *)data->kp_kallsyms_lookup_name(name);
}
EXPORT_SYMBOL_GPL(osvelte_kallsyms_lookup_name);
