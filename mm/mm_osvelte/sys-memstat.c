// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#include <linux/fdtable.h>
#include <linux/seq_file.h>
#include <trace/hooks/mm.h>

#include "sys-memstat.h"

static struct proc_dir_entry *mtrack_procs[MTRACK_MAX];
static struct mtrack_debugger *mtrack_debugger[MTRACK_MAX];

void unregister_mtrack_debugger(enum mtrack_type type,
				struct mtrack_debugger *debugger)
{
	mtrack_debugger[type] = NULL;
}
EXPORT_SYMBOL_GPL(unregister_mtrack_debugger);

int register_mtrack_debugger(enum mtrack_type type,
			     struct mtrack_debugger *debugger)
{
	if (!debugger)
		return -EINVAL;

	if (mtrack_debugger[type])
		return -EEXIST;

	mtrack_debugger[type] = debugger;
	return 0;
}
EXPORT_SYMBOL_GPL(register_mtrack_debugger);

int register_mtrack_procfs(enum mtrack_type t, const char *name, umode_t mode,
			   const struct proc_ops *proc_ops, void *data)
{
	struct proc_dir_entry *entry;

	if (!mtrack_procs[t])
		return -EBUSY;

	entry = proc_create_data(name, mode, mtrack_procs[t], proc_ops, data);
	if (!entry)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL_GPL(register_mtrack_procfs);

void unregister_mtrack_procfs(enum mtrack_type t, const char *name)
{
	if (!unlikely(mtrack_procs[t]))
		return;

	remove_proc_subtree(name, mtrack_procs[t]);
}
EXPORT_SYMBOL_GPL(unregister_mtrack_procfs);
