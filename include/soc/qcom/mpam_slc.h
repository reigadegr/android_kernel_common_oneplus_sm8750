/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _QCOM_MPAM_SLC_H
#define _QCOM_MPAM_SLC_H

struct qcom_slc_gear_val {
	uint32_t gear_val;
} __packed;

enum slc_mon_function {
	CACHE_CAPACITY_CONFIG,
	CASHE_READ_MISS_CONFIG,
};

struct slc_capacity {
	uint32_t num_cache_lines;
	uint32_t cap_enabled;
} __packed;

struct slc_read_miss_cntr {
	uint64_t rd_misses;
	uint32_t cntr_index;
	uint32_t miss_enabled;
} __packed;

struct slc_partid_info {
	uint32_t client_id;
	uint32_t part_id;
} __packed;

struct slc_client_monitor_stats {
	struct slc_capacity slc_lines;
	struct slc_read_miss_cntr rd_miss_stats;
} __packed;

struct qcom_slc_mon_data {
	struct slc_partid_info part_info;
	struct slc_capacity cap_stats;
	struct slc_read_miss_cntr rd_miss_stats;
} __packed;


#define SLC_NUM_PARTIDS		5
struct qcom_slc_mon_mem {
	uint32_t match_seq;
	uint16_t msc_id;
	uint16_t num_active_mon;
	struct qcom_slc_mon_data data[SLC_NUM_PARTIDS];
	uint64_t last_capture_time;
} __packed;

struct capacity_info  {
	uint32_t num_cache_lines;
	uint64_t last_capture_time;
} __packed;

struct miss_info  {
	uint64_t num_rd_misses;
	uint64_t last_capture_time;
} __packed;

union mon_values {
	struct capacity_info capacity;
	struct miss_info misses;
};

struct qcom_slc_mon_data_val {
	struct slc_partid_info part_info;
	uint32_t num_cache_lines;
	uint64_t rd_misses;
} __packed;

struct qcom_msc_slc_mon_val {
	struct qcom_slc_mon_data_val data[SLC_NUM_PARTIDS];
	uint64_t last_capture_time;
} __packed;

struct slc_mon_config_val {
	uint32_t slc_mon_function;
	uint32_t enable;
} __packed;

struct slc_parid_config {
	struct msc_query query;
	struct qcom_slc_gear_val gear_config;
} __packed;

struct slc_mon_capability {
	uint32_t read_miss_config_available;
	uint32_t capacity_config_available;
};

struct slc_mon_configured {
	uint32_t read_miss_configured;
	uint32_t capacity_configured;
};

struct slc_mon_config {
	struct msc_query query;
	struct slc_mon_config_val config;
} __packed;

#define MAX_NUM_GEARS		3
#define MAX_PART_ID		10
#define SLC_INVALID_PARTID      (1 << 16) - 1
struct slc_partid_capability {
	uint8_t part_id;
	uint8_t num_gears;
	uint8_t part_id_gears[MAX_NUM_GEARS];
} __packed;

struct slc_client_info {
	uint16_t client_id;
	uint16_t num_part_id;
} __packed;

struct slc_client_capability {
	struct slc_client_info client_info;
	struct slc_partid_capability *slc_partid_cap;
	uint8_t enabled;
	const char *client_name;
} __packed;

struct qcom_slc_capability {
	uint32_t num_clients;
	struct slc_client_capability *slc_client_cap;
	struct slc_mon_capability slc_mon_list;
	struct slc_mon_configured slc_mon_configured;
} __packed;

enum slc_clients_id {
	APPS,
	GPU,
	NSP,
	SLC_CLIENT_MAX,
};

enum gear_val {
	GEAR_HIGH,
	GEAR_LOW,
	GEAR_BYPASS,
	GEAR_MAX,
};

static char gear_index[][25] = {
	"SLC_GEAR_HIGH",
	"SLC_GEAR_LOW",
	"SLC_GEAR_BYPASS",
	"",
};

#endif /* _QCOM_MPAM_SLC_H */
