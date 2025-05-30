/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2020 Oplus. All rights reserved.
 */

#ifndef _FRAME_DEBUG_H
#define _FRAME_DEBUG_H

/*debug level for fbg*/
#define DEBUG_SYSTRACE (1 << 0)
#define DEBUG_FTRACE   (1 << 1)
#define DEBUG_KMSG     (1 << 2)
#define DEBUG_VERBOSE  (1 << 10)

void val_systrace_c(int grp_id, unsigned long val, char *msg);
void frame_min_util_systrace_c(int grp_id, int util);
void cfs_policy_util_systrace_c(int grp_id, unsigned long util);
void cfs_curr_util_systrace_c(int grp_id, unsigned long util);
void rt_policy_util_systrace_c(int grp_id, unsigned long util);
void rt_curr_util_systrace_c(int grp_id, unsigned long util);
void raw_util_systrace_c(unsigned int cpu, unsigned long util);
void max_grp_systrace_c(unsigned int cpu, int grp_id);
void pref_cpus_systrace_c(int grp_pid, unsigned int cpu);
void avai_cpus_systrace_c(int grp_pid, unsigned int cpu);
void query_cpus_systrace_c(int grp_id, unsigned int cpu);
void sf_zone_systrace_c(int grp_id, unsigned int zone);
void def_zone_systrace_c(int grp_id, unsigned int zone);
void fbg_state_systrace_c(unsigned int cpu, u16 fbg_state);
void cpu_util_systrace_c(int grp_id, unsigned long util, unsigned int cpu, char *msg);
void fbg_info_systrace_c(int grp_id, int tsk_id, char *msg);

#endif /* _FRAME_DEBUG_H */
