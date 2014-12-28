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

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cpu.h>

#include "buffer.h"
#include "src_map.h"
#include "met_struct.h"
#include "v7_pmu.h"
#include "trace.h"

#ifdef CONFIG_CPU_FREQ
#include "power.h"

extern volatile int do_dvfs;
#endif

void add_sample(struct pt_regs *regs, int cpu)
{
	struct met_cpu_struct *met_cpu_ptr;
	struct sample *s;
	unsigned long flags;

	met_cpu_ptr = &per_cpu(met_cpu, cpu);

	s = (struct sample *) kmem_cache_alloc(met_cpu_ptr->cachep, GFP_ATOMIC);
	if (s == NULL) {
		printk("WHY?\n");
	} else {
		s->cpu = cpu;
		s->stamp = cpu_clock(cpu);
		s->task = current;
		s->count = v7_pmu_polling(s->pmu_value);

		if (regs) {
			s->pc = profile_pc(regs);
		} else {
			s->pc = 0x0;
		}

		INIT_LIST_HEAD(&s->list);
	}

	spin_lock_irqsave(&met_cpu_ptr->list_lock, flags);
	list_add_tail(&s->list, &met_cpu_ptr->sample_head);
	spin_unlock_irqrestore(&met_cpu_ptr->list_lock, flags);
}

void cpu_sync_sample(void *data)
{
	struct sample *s = (struct sample *)data;
	mp_cp_ptr(s->stamp, s->task, s->pc, s->cookie, s->off, s->count, s->pmu_value);
}

void sync_samples(int cpu)
{
	struct met_cpu_struct *met_cpu_ptr;
	unsigned long flags;
	struct sample *s, *next;
	struct list_head head;

#ifdef CONFIG_CPU_FREQ
	static int force_power_num = 0;

	if (do_dvfs != 0) {
		if (cpu == 0) {
			force_power_num++;
			if (force_power_num == 50) {
				force_power_log(POWER_LOG_ALL);
				force_power_num = 0;
			}
		}
	}
#endif

	met_cpu_ptr = &per_cpu(met_cpu, cpu);

	INIT_LIST_HEAD(&head);

	mutex_lock(&met_cpu_ptr->list_sync_lock);

	spin_lock_irqsave(&met_cpu_ptr->list_lock, flags);
	list_splice_init(&met_cpu_ptr->sample_head, &head);
	spin_unlock_irqrestore(&met_cpu_ptr->list_lock, flags);

	list_for_each_entry_safe(s, next, &head, list) {
		list_del(&s->list);

		if (s->pc) {
			struct mm_struct *mm;

			mm = take_tasks_mm(s->task);
			if (mm != NULL) {
				s->cookie = lookup_dcookie(mm, s->pc, &(s->off));
				release_mm(mm);
			} else {
				s->cookie = NO_COOKIE;
				s->off = 0;
			}
		} else {
			s->cookie = NO_COOKIE;
			s->off = 0;
		}
		smp_call_function_single(cpu, cpu_sync_sample, s, 1);
		kmem_cache_free(met_cpu_ptr->cachep, s);
	}
	mutex_unlock(&met_cpu_ptr->list_sync_lock);

	mark_done(cpu);
}

void sync_all_samples(void)
{
	int cpu;
	for_each_possible_cpu(cpu) {
		sync_samples(cpu);
	}
}
