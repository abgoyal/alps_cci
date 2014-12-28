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

#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/cpu.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/profile.h>
#include <linux/dcache.h>
#include <linux/types.h>
#include <linux/dcookies.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <asm/irq_regs.h>

#include "buffer.h"
#include "met_struct.h"
#include "met_drv.h"

DEFINE_PER_CPU(struct met_cpu_struct, met_cpu);
//struct met_cpu_struct met_sys;

int fs_reg(void);
void fs_unreg(void);

static int __init met_pmu_init(void)
{
	int cpu;
	struct met_cpu_struct *met_cpu_ptr;

	for_each_possible_cpu(cpu) {
		met_cpu_ptr = &per_cpu(met_cpu, cpu);

		snprintf(&(met_cpu_ptr->name[0]), sizeof(met_cpu_ptr->name), "met%02d", cpu);
		met_cpu_ptr->cachep =
			kmem_cache_create(met_cpu_ptr->name, sizeof(struct sample), __alignof__(struct sample), 0, NULL);

		spin_lock_init(&met_cpu_ptr->list_lock);
		mutex_init(&met_cpu_ptr->list_sync_lock);
		INIT_LIST_HEAD(&met_cpu_ptr->sample_head);
		met_cpu_ptr->cpu = cpu;
	}

	fs_reg();
	return 0;
}

static void __exit met_pmu_exit(void)
{
	int cpu;

	fs_unreg();
	for_each_possible_cpu(cpu) {
		kmem_cache_destroy(per_cpu(met_cpu, cpu).cachep);
	}
}

module_init(met_pmu_init);
module_exit(met_pmu_exit);

MODULE_AUTHOR("DT_DM5");
MODULE_DESCRIPTION("MET_CORE");
MODULE_LICENSE("GPL");