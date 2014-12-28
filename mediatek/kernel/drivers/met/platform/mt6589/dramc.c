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

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>

#include "core/met_drv.h"
#include "core/trace.h"

#include "mt_typedefs.h"
#include "mt_reg_base.h"
#include "mt_emi_bm.h"
#include "sync_write.h"
#include "plf_trace.h"
//#include "dramc.h"

extern struct metdevice met_dramc;
/*
static struct kobject *kobj_dramc = NULL;
static ssize_t evt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t evt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n);
static struct kobj_attribute evt_attr = __ATTR(evt, 0644, evt_show, evt_store);

static ssize_t evt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int i=0;
	int j;
	for (j=0; j<MCI_DESC_COUNT; j++) {
		i += snprintf(buf+i, PAGE_SIZE-i, "0x%x: %s\n", dramc_desc[j].event, dramc_desc[j].name);
	}
	i += snprintf(buf+i, PAGE_SIZE-i, "\nCurrent counter0=0x%x, counter1=0x%x\n\n", MCI_GetEvent(0), MCI_GetEvent(1));

	return i;
}

static ssize_t evt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned int evt0, evt1;
	if (sscanf(buf, "%x %x", &evt0, &evt1) != 2)
		return -EINVAL;

	MCI_Event_Set(evt0, evt1);
	//MCI_Event_Read();
	return n;
}
*/

/*
static void dramc_value_init(void)
{
}
*/

static void dramc_init(void)
{
}

static void dramc_start(void)
{
}

static void dramc_stop(void)
{
}

static int do_dramc(void)
{
	return met_dramc.mode;
}

static unsigned int dramc_polling(unsigned int *value)
{
	int j = -1;

	value[++j] = DRAMC_GetPageHitCount(DRAMC_ALL);
	value[++j] = DRAMC_GetPageMissCount(DRAMC_ALL);
	value[++j] = DRAMC_GetInterbankCount(DRAMC_ALL);
	value[++j] = DRAMC_GetIdleCount();

	return j+1;
}

static void dramc_uninit(void)
{
}

/*
static int met_dramc_create(struct kobject *parent)
{
	int ret = 0;

	kobj_dramc = parent;

	ret = sysfs_create_file(kobj_dramc, &evt_attr.attr);
	if (ret != 0) {
		pr_err("Failed to create evt in sysfs\n");
		return ret;
	}
	dramc_value_init();
    return ret;
}

static void met_dramc_delete(void)
{
	sysfs_remove_file(kobj_dramc, &evt_attr.attr);
	kobj_dramc = NULL;
}
*/

static void met_dramc_start(void)
{
	if (do_dramc()) {
		dramc_init();
		dramc_stop();
		dramc_start();
	}
}

static void met_dramc_stop(void)
{
	if (do_dramc()) {
		dramc_stop();
		dramc_uninit();
	}

}

static void met_dramc_polling(unsigned long long stamp, int cpu)
{
	unsigned char count;
	unsigned int dramc_value[4];

	if (do_dramc()) {
		count = dramc_polling(dramc_value);
		if (count) {
			ms_dramc(stamp, count, dramc_value);
		}
	}
}

static char header[] =
"met-info [000] 0.0: ms_ud_sys_header: ms_dramc,PageHit,PageMiss,InterBank,Idle,x,x,x,x\n";
static char help[] = "  --dramc                             monitor DRAMC\n";

static int dramc_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int dramc_print_header(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, header);
}

struct metdevice met_dramc = {
	.name = "dramc",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	//.create_subfs = met_dramc_create,
	//.delete_subfs = met_dramc_delete,
	.cpu_related = 0,
	.start = met_dramc_start,
	.stop = met_dramc_stop,
	.polling_interval = 0,//ms
	.timed_polling = met_dramc_polling,
	.tagged_polling = met_dramc_polling,
	.print_help = dramc_print_help,
	.print_header = dramc_print_header,
};

