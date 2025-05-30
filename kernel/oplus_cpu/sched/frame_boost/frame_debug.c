// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020 Oplus. All rights reserved.
 */

#include <linux/string.h>
#include <linux/kernel.h>

static noinline int tracing_mark_write(const char *buf)
{
	trace_printk(buf);
	return 0;
}

void val_systrace_c(int grp_id, unsigned long val, char *msg)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|grp[%d]_%s|%lu\n", grp_id, msg, val);
	tracing_mark_write(buf);
}

#if IS_ENABLED(CONFIG_OPLUS_FEATURE_FRAME_BOOST_MULTI)
void frame_min_util_systrace_c(int grp_id, int util)
{
	val_systrace_c(grp_id, util, "frame_min_util");
}

void pref_cpus_systrace_c(int grp_id, unsigned int cpu)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|grp[%d]_02_cls_pref|%d\n", grp_id, cpu);
	tracing_mark_write(buf);
}

void avai_cpus_systrace_c(int grp_id, unsigned int cpu)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|grp[%d]_02_cls_avai|%d\n", grp_id, cpu);
	tracing_mark_write(buf);
}

void sf_zone_systrace_c(int grp_id, unsigned int zone)
{
	val_systrace_c(grp_id, zone, "01_frame_zone_sf");
}

void def_zone_systrace_c(int grp_id, unsigned int zone)
{
	val_systrace_c(grp_id, zone, "01_frame_zone_def");
}

void fbg_info_systrace_c(int grp_id, int tsk_id, char *msg)
{
		val_systrace_c(grp_id, tsk_id, msg);
}

#else
void frame_min_util_systrace_c(int grp_id, int util)
{
	static int prev_util = 0;
	if (prev_util != util) {
		val_systrace_c(grp_id, util, "frame_min_util");
		prev_util = util;
	}
}

void cfs_policy_util_systrace_c(int grp_id, unsigned long util)
{
	static unsigned long prev_util;

	if (prev_util != util) {
		val_systrace_c(grp_id, util, "cfs_policy_util");
		prev_util = util;
	}
}

void cfs_curr_util_systrace_c(int grp_id, unsigned long util)
{
	static unsigned long prev_util;

	if (prev_util != util) {
		val_systrace_c(grp_id, util, "cfs_curr_util");
		prev_util = util;
	}
}

void rt_policy_util_systrace_c(int grp_id, unsigned long util)
{
	static unsigned long prev_util;

	if (prev_util != util) {
		val_systrace_c(grp_id, util, "rt_policy_util");
		prev_util = util;
	}
}

void rt_curr_util_systrace_c(int grp_id, unsigned long util)
{
	static unsigned long prev_util;

	if (prev_util != util) {
		val_systrace_c(grp_id, util, "rt_curr_util");
		prev_util = util;
	}
}

void raw_util_systrace_c(unsigned int cpu, unsigned long util)
{
	char buf[256];
	static unsigned long prev_util;

	if (prev_util != util) {
		snprintf(buf, sizeof(buf), "C|10000|cls[%d]_raw_util|%lu\n", cpu, util);
		tracing_mark_write(buf);
		prev_util = util;
	}
}

void pref_cpus_systrace_c(int grp_id, unsigned int cpu)
{
	char buf[256];
	static unsigned int prev_cpu;

	if (prev_cpu != cpu) {
		snprintf(buf, sizeof(buf), "C|10000|grp[%d]_02_cls_pref|%d\n", grp_id, cpu);
		tracing_mark_write(buf);
		prev_cpu = cpu;
	}
}

void avai_cpus_systrace_c(int grp_id, unsigned int cpu)
{
	char buf[256];
	static unsigned int prev_cpu;

	if (prev_cpu != cpu) {
		snprintf(buf, sizeof(buf), "C|10000|grp[%d]_02_cls_avai|%d\n", grp_id, cpu);
		tracing_mark_write(buf);
		prev_cpu = cpu;
	}
}

void query_cpus_systrace_c(int grp_id, unsigned int cpu)
{
	static unsigned int prev_cpu;

	if (prev_cpu != cpu) {
		val_systrace_c(grp_id, cpu, "query_cpus");
		prev_cpu = cpu;
	}
}

void sf_zone_systrace_c(int grp_id, unsigned int zone)
{
	static unsigned int prev_zone;

	if (prev_zone != zone) {
		val_systrace_c(grp_id, zone, "01_frame_zone_sf");
		prev_zone = zone;
	}
}

void def_zone_systrace_c(int grp_id, unsigned int zone)
{
	static unsigned int prev_zone;

	if (prev_zone != zone) {
		val_systrace_c(grp_id, zone, "01_frame_zone_def");
		prev_zone = zone;
	}
}

void fbg_info_systrace_c(int grp_id, int tsk_id, char *msg)
{
	static int prev_tsk_id;

	if (prev_tsk_id != tsk_id) {
		val_systrace_c(grp_id, tsk_id, msg);
		prev_tsk_id = tsk_id;
	}
}
#endif

void fbg_state_systrace_c(unsigned int cpu, u16 fbg_state)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|fbg_state_cpu%d|0x%x\n", cpu, fbg_state);
	tracing_mark_write(buf);
}

void cpu_util_systrace_c(int grp_id, unsigned long util, unsigned int cpu, char *msg)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|grp[%d]_cls[%d]_%s|%lu\n", grp_id, cpu, msg, util);
	tracing_mark_write(buf);
}

void max_grp_systrace_c(unsigned int cpu, int grp_id)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "C|10000|cls[%d]_max_grp_id|%d\n", cpu, grp_id);
	tracing_mark_write(buf);
}
