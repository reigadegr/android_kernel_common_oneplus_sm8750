/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _QCOM_LLCC_HEURISTICS_H
#define _QCOM_LLCC_HEURISTICS_H

#define SCID_HEURISTICS_SCMI_STR	0x4845555253434944 /* "HEURSCID" */
#define CD_MAX				2

enum llcc_set_attribute {
	HEURISTICS_INIT,
	HEURISTICS_UPDATE,
	SCID_ACTIVATION_CONTROL,
};

enum llcc_get_attribute {
	HEURISTICS_SCID_STATS,
	HEURISTICS_TRIGGER_STATUS,
};

/* HEURISTICS_TRIGGER_STATUS */
struct heuristics_scid_triger_stats {
	uint32_t ss_lpm_entry_counter;
	uint32_t clkdom0_act_trigger;
	uint32_t clkdom1_act_trigger;
} __packed;

/* HEURISTICS_SCID_STATS */
struct heuristics_scid_stats {
	uint32_t heuristics_scid_act_counter;
	uint32_t heuristics_scid_deact_counter;
	uint64_t heuristics_scid_act_residency;
} __packed;

/* HEURISTICS_INIT */
struct scid_heuristics_params {
	uint32_t heuristics_scid;
	uint32_t freq_idx[CD_MAX];
	uint32_t freq_idx_residency[CD_MAX];
	uint32_t thread_interval;
	uint32_t scid_heuristics_enabled;
} __packed;

struct scid_heuristics_data {
	struct scid_heuristics_params params;
} __packed;

#endif /* _QCOM_LLCC_HEURISTICS_H */
