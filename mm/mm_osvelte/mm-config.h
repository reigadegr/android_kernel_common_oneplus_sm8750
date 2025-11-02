/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2024 Oplus. All rights reserved.
 * this referenced by android cutil/trace.h
 */
#ifndef _OSVELTE_MM_CONFIG_H
#define _OSVELTE_MM_CONFIG_H

extern void *oplus_read_mm_config(const char *module_name);
extern bool oplus_test_mm_feature_disable(unsigned long nr);
#endif /* _OSVELTE_MM_CONFIG_H */
