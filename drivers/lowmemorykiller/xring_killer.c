// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring Mem adapter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/sched.h>
#include <linux/oom.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>

/* copy from oom_kill.c - find_lock_task_mm */
struct task_struct *find_task_mm(struct task_struct *p)
{
	struct task_struct *t = NULL;

	rcu_read_lock();

	for_each_thread(p, t) {
		task_lock(t);
		if (likely(t->mm))
			goto found;
		task_unlock(t);
	}
	t = NULL;
found:
	rcu_read_unlock();

	return t;
}
EXPORT_SYMBOL(find_task_mm);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring MEM_ADAPTER Driver");
MODULE_LICENSE("GPL v2");
