/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2021 Oplus. All rights reserved.
 */
#ifndef _OSVELTE_COMMON_H
#define _OSVELTE_COMMON_H

#define KMODULE_NAME "oplus_bsp_mm_osvelte"

#define DEV_NAME "osvelte"

#define DEV_PATH "/dev/" DEV_NAME

#define OSVELTE_LOG_TAG DEV_NAME

/* declare page-flags here */
#define PG_ezreclaimable (PG_oem_reserved_1)

enum oplus_mm_symbol {
	OPLUS_MM_KOBJ,
	OPLUS_TASK_EZRECLAIMD,
	OMS_END,
};

/* common ioctl for userspace */
#define __COMMONIO 0xFA
#define CMD_OSVELTE_GET_VERSION		_IO(__COMMONIO, 1)
#define CMD_OSVELTE_SET_SCENE		_IO(__COMMONIO, 2)
#define CMD_OSVELTE_CLEAR_SCENE		_IO(__COMMONIO, 3)

/* kgsl.c use osvelte_info */
#define osvelte_info(fmt, ...)      \
	pr_info(OSVELTE_LOG_TAG ": " fmt, ##__VA_ARGS__)

#define osvelte_err(fmt, ...)      \
	pr_err(OSVELTE_LOG_TAG ": " fmt, ##__VA_ARGS__)

extern struct kobject *oplus_mm_kobj;
extern void osvelte_register_symbol(enum oplus_mm_symbol sym, void *data);
extern void *osvelte_read_symbol(enum oplus_mm_symbol sym, bool atomic);
extern bool osvelte_test_scene(unsigned long nr);
extern void *osvelte_kallsyms_lookup_name(const char *name);
#endif /* _OSVELTE_COMMON_H */
