/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2021 Oplus. All rights reserved.
 */
#ifndef _OSVELTE_SYS_MEMSTAT_H
#define _OSVELTE_SYS_MEMSTAT_H

#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#ifdef CONFIG_CONT_PTE_HUGEPAGE
#include "../../../mm/chp_ext.h"
#endif /* CONFIG_CONT_PTE_HUGEPAGE */

enum mtrack_type {
	MTRACK_GPU,
	MTRACK_MAX
};

enum mtrack_subtype {
	MTRACK_GPU_TOTAL,
	MTRACK_GPU_PROC_KERNEL,
	MTRACK_SUBTYPE_MAX
};

struct mtrack_debugger {
	long (*mem_usage)(enum mtrack_subtype type);
	long (*pid_mem_usage)(enum mtrack_subtype type, pid_t pid);
	void (*dump_usage_stat)(bool verbose);
};

int register_mtrack_debugger(enum mtrack_type type,
			     struct mtrack_debugger *debugger);
void unregister_mtrack_debugger(enum mtrack_type type,
				struct mtrack_debugger *debugger);
int register_mtrack_procfs(enum mtrack_type t, const char *name, umode_t mode,
			   const struct proc_ops *proc_ops, void *data);
void unregister_mtrack_procfs(enum mtrack_type t, const char *name);

inline long read_mtrack_mem_usage(enum mtrack_type t, enum mtrack_subtype s);
inline long read_pid_mtrack_mem_usage(enum mtrack_type t, enum mtrack_subtype s,
				      pid_t pid);
inline void dump_mtrack_usage_stat(enum mtrack_type t, bool verbose);

#endif /* _OSVELTE_SYS_MEMSTAT_H */
