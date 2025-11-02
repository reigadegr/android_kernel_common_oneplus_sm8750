// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2023 Oplus. All rights reserved.
 */

#include <linux/proc_fs.h>

#include "sys-memstat.h"

inline long read_mtrack_mem_usage(enum mtrack_type t,
				  enum mtrack_subtype s)
{
	return 0;
}
inline long read_pid_mtrack_mem_usage(enum mtrack_type t,
				      enum mtrack_subtype s,
				      pid_t pid)
{
	return 0;
}
inline void dump_mtrack_usage_stat(enum mtrack_type t, bool verbose)
{
}
