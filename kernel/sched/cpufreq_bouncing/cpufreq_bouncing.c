/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2019-2021 Oplus. All rights reserved.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/topology.h>
#include <linux/pm_opp.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/sched/sysctl.h>
#include <linux/sched/walt.h>
#include <linux/version.h>
#include <linux/freezer.h>
#include <linux/workqueue.h>

#define NSEC_TO_MSEC(val) (val / NSEC_PER_MSEC)
#define MSEC_TO_NSEC(val) (val * NSEC_PER_MSEC)
#define MSEC_TO_USEC(val) (val * USEC_PER_MSEC)

#define NR_FREQ 48
/* New Qcom SOC with 4 cpufreq cluster, use kernel6.1 version */
#define NR_CLUS_MAX 4
#define NR_CORE_MAX 8

/* cluster based */
struct cpufreq_bouncing {
	int first_cpu;

	/* statistics */
	u64 last_ts;
	u64 last_freq_update_ts;
	u64 acc;

	/* restriction */
	int limit_freq;
	int limit_level;
	u64 limit_thres;

	/* freqs */
	int freq_sorting;
	int freq_levels;
	unsigned int freqs[NR_FREQ]; /* quick mapping */

	/* trace info */
	long long freqs_resident[NR_FREQ]; /* for record how long freqs stay */

	/* config */
	bool enable;
	int cur_level;

	/* config: ceil */
	int max_level;
	int down_speed;
	s64 down_limit_ns;
	unsigned int max_freq;

	/* config: floor */
	int min_level;
	int up_speed;
	s64 up_limit_ns;
	unsigned int min_freq;

	/*
	 * check current limitation
	 * if limitation higher than target, not count
	 */

	/* control freq boundary */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
	struct freq_qos_request qos_req;
	struct work_struct qos_work;
#endif
	bool ctl_inited;
} cb_stuff[NR_CLUS_MAX] = {
	/* silver */
	{
		.first_cpu     = -1,
		.enable        = false,
		.freqs_resident[0 ... NR_FREQ - 1] = -1,
		.freqs[0 ... NR_FREQ - 1] = UINT_MAX,
	},
	/* gold 3-core */
	{
		.first_cpu     = -1,
		.enable        = false,
		.down_limit_ns = 50 * NSEC_PER_MSEC,
		.up_limit_ns   = 50 * NSEC_PER_MSEC,
		.limit_thres   = 30 * NSEC_PER_MSEC,
		.limit_freq    = 0,
		.limit_level   = 0,
		.down_speed    = 0,
		.up_speed      = 0,
		.freqs_resident[0 ... NR_FREQ - 1] = -1,
		.freqs[0 ... NR_FREQ - 1] = UINT_MAX,
	},
	/* gold 2-core */
	{
		.first_cpu     = -1,
		.enable        = false,
		.down_limit_ns = 50 * NSEC_PER_MSEC,
		.up_limit_ns   = 50 * NSEC_PER_MSEC,
		.limit_thres   = 30 * NSEC_PER_MSEC,
		.limit_freq    = 0,
		.limit_level   = 0,
		.down_speed    = 0,
		.up_speed      = 0,
		.freqs_resident[0 ... NR_FREQ - 1] = -1,
		.freqs[0 ... NR_FREQ - 1] = UINT_MAX,
	},
	/* gold prime */
	{
		.first_cpu     = -1,
		.enable        = false,
		.down_limit_ns = 50 * NSEC_PER_MSEC,
		.up_limit_ns   = 50 * NSEC_PER_MSEC,
		.limit_thres   = 30 * NSEC_PER_MSEC,
		.limit_freq    = 0,
		.limit_level   = 0,
		.down_speed    = 0,
		.up_speed      = 0,
		.freqs_resident[0 ... NR_FREQ - 1] = -1,
		.freqs[0 ... NR_FREQ - 1] = UINT_MAX,
	}
};

/* core boost */
static bool last_core_boost;
static atomic_t is_cb_ceiling_free;
static struct workqueue_struct *cb_qos_wq;

static void cb_reset_qos(int cpu)
{
	struct cpufreq_bouncing *cb = &cb_stuff[cpu];

	if (!cb->ctl_inited)
		return;
}

static bool enable = false;

static bool debug, init_complete;

static unsigned int decay = 80;

static DEFINE_PER_CPU(struct cpufreq_bouncing*, cbs);

static inline struct cpufreq_bouncing *cb_get(int cpu)
{
	struct cpufreq_bouncing *cb;

	if (cpu < 0 || cpu >= NR_CORE_MAX)
		return NULL;

	cb = per_cpu(cbs, cpu);

	if (unlikely(!cb))
		return NULL;

	if (unlikely(!cb->ctl_inited))
		return NULL;

	if (!enable || !cb->enable)
		return NULL;

	return cb;
}

void cb_ceiling_free(bool ceiling_free_enable)
{
	if (!enable)
		return;

	if (atomic_read(&is_cb_ceiling_free) == ceiling_free_enable)
		return;

	atomic_set(&is_cb_ceiling_free, ceiling_free_enable);

	for (int i = 0; i < NR_CLUS_MAX; ++i) {
		struct cpufreq_bouncing *cb = &cb_stuff[i];
		cb = cb_get(cb->first_cpu);
		if (!cb)
			continue;

		if (ceiling_free_enable)
			cb_reset_qos(i);
		else if (likely(cb_qos_wq))
			queue_work(cb_qos_wq, &cb->qos_work);
	}
}
EXPORT_SYMBOL(cb_ceiling_free);

static inline bool clus_isolated(struct cpufreq_policy *pol)
{
	cpumask_t active;

	cpumask_and(&active, pol->related_cpus, cpu_active_mask);
	return !cpumask_weight(&active);
}

void cb_update(struct cpufreq_policy *pol, u64 time)
{
	struct cpufreq_bouncing *cb;
	u64 delta, update_delta;
	int cpu, prev_level;
	bool min_over_target_freq, isolated;

	if (unlikely(!pol->fast_switch_enabled))
		return;

	if (!enable || !init_complete)
		return;

	cpu = cpumask_first(pol->related_cpus);
	cb = cb_get(cpu);

	if (unlikely(!cb))
		return;

	/* check isolated status */
	isolated = clus_isolated(pol);

	/* check current min_freq and limit target freq, if min_freq large than limited, reset acc */
	min_over_target_freq = pol->min >= cb->limit_freq;
	if (min_over_target_freq || isolated)
		cb->acc = 0;

	/* for first update */
	if (unlikely(!cb->last_ts))
		cb->last_ts = cb->last_freq_update_ts = time;

	/*
	 * not count flag only affects to delta.
	 * keep update_delta to let limit_freq has time to restore
	 */
	delta = ((min_over_target_freq || isolated) ? 0 : (time - cb->last_ts));
	update_delta = time - cb->last_freq_update_ts;

	/* check cpufreq */
	if (pol->cur >= cb->limit_freq) {
		/* accumulate delta time */
		cb->acc += delta;
	} else {
		/* decay accumulate time */
		cb->acc = cb->acc * decay / 100;
	}

	/* check if need to update limitation */
	prev_level = cb->cur_level;
	if (cb->acc >= cb->limit_thres) {
		/* check last update */
		if (update_delta >= cb->down_limit_ns) {
			cb->cur_level = max(prev_level - cb->down_speed, cb->limit_level);
			cb->last_freq_update_ts = time;
			cb->freqs_resident[prev_level] += update_delta;
		}
	} else {
		/* check last update */
		if (update_delta >= cb->up_limit_ns) {
			cb->cur_level = min(prev_level + cb->up_speed, cb->max_level);
			cb->last_freq_update_ts = time;
			cb->freqs_resident[prev_level] += update_delta;
		}
	}

	/* when min bar is higher than cb limit, unlock immediately */
	if (min_over_target_freq || isolated)
		cb->cur_level = cb->max_level;

	/* update core_ctl boost status */
	/* cb_core_boost(time); */

	if (debug)
		pr_info("cpu %d update: ts now %llu last %llu last_update %llu delta %llu update_d %llu cur %u acc %llu cur_level %id "
			"min_over_target_freq %d isolated %d last_core_boost %d\n",
			cpu,
			NSEC_TO_MSEC(time),
			NSEC_TO_MSEC(cb->last_ts),
			NSEC_TO_MSEC(cb->last_freq_update_ts),
			NSEC_TO_MSEC(delta),
			NSEC_TO_MSEC(update_delta),
			pol->cur,
			NSEC_TO_MSEC(cb->acc),
			cb->cur_level,
			min_over_target_freq,
			isolated,
			last_core_boost);

	cb->last_ts = time;
}
EXPORT_SYMBOL(cb_update);
