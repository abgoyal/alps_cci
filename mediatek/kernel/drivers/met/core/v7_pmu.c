/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <asm/page.h>
#include <linux/slab.h>

#include "trace.h"
#include "v7_pmu.h"
#include "v7_pmu_hw.h"
#include "v7_pmu_name.h"

static int nr_counters = 0;
static int ctrl = 0;

static struct kobject *kobj_cpu = NULL;
static struct met_pmu *pmu = NULL;

struct chip_pmu {
	enum ARM_TYPE type;
	struct pmu_desc *desc;
	unsigned int count;
};

static struct chip_pmu chips[] ={
	{ CORTEX_A7, a7_pmu_desc, A7_PMU_DESC_COUNT },
	{ CORTEX_A9, a9_pmu_desc, A9_PMU_DESC_COUNT },
	{ CORTEX_A15, a7_pmu_desc, A7_PMU_DESC_COUNT },
};
static struct chip_pmu chip_unknown = { CHIP_UNKNOWN, NULL, 0 };

#define CHIP_PMU_COUNT (sizeof(chips) / sizeof(struct chip_pmu))

static struct chip_pmu *chip;

static inline struct met_pmu *lookup_pmu(struct kobject *kobj)
{
	int i;
	for (i=0; i<nr_counters; i++) {
		if (pmu[i].kobj_cpu_pmu == kobj) {
			return &pmu[i];
		}
	}
	return NULL;
}

static ssize_t count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", nr_counters-1);
}

static ssize_t count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	return -EINVAL;
}

static ssize_t ctrl_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ctrl);
}

static ssize_t ctrl_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	int tmp;

	if (sscanf(buf, "%d", &tmp) != 1) {
		return -EINVAL;
	}
	switch (tmp) {
	case 0:
		mp_cp_ptr = &mp_cp;
		break;
	case 1:
		mp_cp_ptr = &mp_cp2;
		break;
	default:
		return -EINVAL;
	}
	ctrl = tmp;

	return n;
}

static ssize_t event_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);
	if (p != NULL) {
		return snprintf(buf, PAGE_SIZE, "0x%hx\n", p->event);
	}
	return -EINVAL;
}

static ssize_t event_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	int i;
	struct met_pmu *p = lookup_pmu(kobj);
	unsigned short event;

	if (p != NULL) {
		if (sscanf(buf, "0x%hx", &event) != 1) {
			return -EINVAL;
		}

		if (p == &(pmu[nr_counters-1])) { // cycle counter
			if (event != 0xff) {
				return -EINVAL;
			}
		} else {
			for (i=0; i<chip->count; i++) {
				if (chip->desc[i].event == event) {
					break;
				}
			}
			if (i == chip->count) {
				return -EINVAL;
			}
		}

		p->event = event;
		return n;
	}
	return -EINVAL;
}

static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);
	if (p != NULL) {
		switch (p->mode) {
		case 0:
			return snprintf(buf, PAGE_SIZE, "%hhd (disabled)\n", p->mode);
		case 1:
			return snprintf(buf, PAGE_SIZE, "%hhd (interrupt)\n", p->mode);
		case 2:
			return snprintf(buf, PAGE_SIZE, "%hhd (polling)\n", p->mode);
		}

	}
	return -EINVAL;
}

static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned char mode;
	struct met_pmu *p = lookup_pmu(kobj);
	if (p != NULL) {
		if (sscanf(buf, "%hhd", &mode) != 1) {
			return -EINVAL;
		}
		if (mode <= 2) {
			p->mode = mode;
			return n;
		}
	}
	return -EINVAL;
}

static ssize_t freq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct met_pmu *p = lookup_pmu(kobj);
	if (p != NULL) {
		return snprintf(buf, PAGE_SIZE, "%ld\n", p->freq);
	}
	return -EINVAL;
}

static ssize_t freq_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	struct met_pmu *p = lookup_pmu(kobj);
	if (p != NULL) {
		if (sscanf(buf, "%ld", &(p->freq)) != 1) {
			return -EINVAL;
		}
		return n;
	}
	return -EINVAL;
}

static struct kobj_attribute count_attr = __ATTR(count, 0644, count_show, count_store);
static struct kobj_attribute ctrl_attr = __ATTR(ctrl, 0644, ctrl_show, ctrl_store);

static struct kobj_attribute event_attr = __ATTR(event, 0644, event_show, event_store);
static struct kobj_attribute mode_attr = __ATTR(mode, 0644, mode_show, mode_store);
static struct kobj_attribute freq_attr = __ATTR(freq, 0644, freq_show, freq_store);

int v7_pmu_reg(struct kobject *parent)
{
	int ret = 0;
	int i;
	char buf[16];
	enum ARM_TYPE type;

	type = armv7_get_ic();
	for (i=0; i<CHIP_PMU_COUNT; i++) {
		if (chips[i].type == type) {
			chip = &(chips[i]);
			break;
		}
	}
	if (i == CHIP_PMU_COUNT) {
		chip = &chip_unknown;
	}

	nr_counters = armv7_pmu_hw_get_counters() + 1;
	pmu = (struct met_pmu *) kzalloc(sizeof(struct met_pmu) * nr_counters, GFP_KERNEL);
	if (pmu == NULL) {
		pr_err("can not create kobject: kobj_cpu\n");
		ret = -ENOMEM;
		goto out;
	}

	kobj_cpu = kobject_create_and_add("cpu", parent);
	if (kobj_cpu == NULL) {
		pr_err("can not create kobject: kobj_cpu\n");
		ret = -ENOMEM;
		goto out;
	}

	ret = sysfs_create_file(kobj_cpu, &count_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create count in sysfs\n");
		goto out;
	}

	ret = sysfs_create_file(kobj_cpu, &ctrl_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create ctrl in sysfs\n");
		goto out;
	}

	for (i=0; i<nr_counters; i++) {
		snprintf(buf, sizeof(buf), "%d", i);
		pmu[i].kobj_cpu_pmu = kobject_create_and_add(buf, kobj_cpu);

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &event_attr.attr);
		if (ret != 0) {
			pr_err("Failed to create event in sysfs\n");
			goto out;
		}

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &mode_attr.attr);
		if (ret != 0) {
			pr_err("Failed to create mode in sysfs\n");
			goto out;
		}

		ret = sysfs_create_file(pmu[i].kobj_cpu_pmu, &freq_attr.attr);
		if (ret != 0) {
			pr_err("Failed to create freq in sysfs\n");
			goto out;
		}
	}

out:
	if (ret != 0) {
		if (pmu != NULL) {
			kfree(pmu);
			pmu = NULL;
		}
	}
	return ret;
}

void v7_pmu_unreg(void)
{
	int i;

	if (kobj_cpu != NULL) {
		for (i=0; i<nr_counters; i++) {
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &event_attr.attr);
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &mode_attr.attr);
			sysfs_remove_file(pmu[i].kobj_cpu_pmu, &freq_attr.attr);
			kobject_del(pmu[i].kobj_cpu_pmu);
		}
		sysfs_remove_file(kobj_cpu, &count_attr.attr);
		sysfs_remove_file(kobj_cpu, &ctrl_attr.attr);
		kobject_del(kobj_cpu);
		kobj_cpu = NULL;
	}

	if (pmu != NULL) {
		kfree(pmu);
		pmu = NULL;
	}
}

void v7_pmu_start(void)
{
	armv7_pmu_hw_start(pmu, nr_counters);
}

void v7_pmu_stop(void)
{
	armv7_pmu_hw_stop(nr_counters);
}

unsigned int v7_pmu_polling(unsigned int *pmu_value)
{
	return armv7_pmu_hw_polling(pmu, nr_counters, pmu_value);
}
