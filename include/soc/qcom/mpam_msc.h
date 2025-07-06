/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _QCOM_MPAM_MSC_H
#define _QCOM_MPAM_MSC_H

#include <linux/qcom_scmi_vendor.h>
#include <linux/scmi_protocol.h>

enum msc_qcom_id {
	L2,
	L3,
	SLC = 2,
	QCOM_MSC_MAX,
};

enum msc_class_type {
	CACHE_TYPE,
	MAX_TYPE,
};

struct qcom_msc_id {
	uint8_t idx;
	uint8_t qcom_msc_class;
	uint16_t qcom_msc_type;
} __packed;

struct msc_query {
	struct qcom_msc_id qcom_msc_id;
	uint16_t part_id;
	uint16_t client_id;
} __packed;


struct qcom_mpam_msc {
	uint32_t msc_id;
	const char *msc_name;
	struct qcom_msc_id qcom_msc_id;
	void *msc_capability;
	struct mpam_msc_ops *ops;
	struct scmi_device *sdev;
	const struct qcom_scmi_vendor_ops *scmi_ops;
	struct scmi_protocol_handle *ph;
	struct device *dev;
	void __iomem *mon_base;
	struct list_head node;
} __packed;

struct mpam_msc_ops {
	int (*set_cache_partition)(struct device *dev, void *msc_partid, void *msc_partconfig);
	int (*get_cache_partition)(struct device *dev, void *msc_partid, void *msc_partconfig);
	int (*get_cache_partition_capability)(struct device *dev, void *msc_partid, void *msc_partconfig);
	int (*reset_cache_partition)(struct device *dev, void *msc_partid, void *msc_partconfig);
	int (*mon_config)(struct device *dev, void *msc_partid, void *msc_partconfig);
	int (*mon_stats_read)(struct device *dev, void *msc_partid, void *mon_val);
};

struct qcom_mpam_msc *qcom_msc_lookup(uint32_t msc_id);
int msc_system_get_device_capability(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_get_partition(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_set_partition(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_reset_partition(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_mon_read_miss_info(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_mon_alloc_info(uint32_t msc_id, void *arg1, void *arg2);
int msc_system_mon_config(uint32_t msc_id, void *arg1, void *arg2);
int attach_dev(struct device *dev, struct qcom_mpam_msc *qcom_msc, uint32_t msc_type);
void detach_dev(struct device *dev, struct qcom_mpam_msc *qcom_msc, uint32_t msc_type);
#endif /* _QCOM_MPAM_MSC_H */
