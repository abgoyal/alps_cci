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

#include <linux/dcache.h>
#include <linux/types.h>
#include <asm-generic/errno.h>
#include <linux/dcookies.h>
#include <linux/profile.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/hardirq.h>

#include "src_map.h"
#include "buffer.h"
#include "util.h"

static cpumask_var_t marked_cpus;
static struct dcookie_user *dcookie = NULL;

static DEFINE_SPINLOCK(task_mortuary);
static DEFINE_MUTEX(die_mutex);
static LIST_HEAD(dying_tasks);
static LIST_HEAD(dead_tasks);

/* Move tasks along towards death. Any tasks on dead_tasks
 * will definitely have no remaining references in any
 * CPU buffers at this point, because we use two lists,
 * and to have reached the list, it must have gone through
 * one full sync already.
 */
static void process_task_mortuary(void)
{
	unsigned long flags;
	LIST_HEAD(local_dead_tasks);
	struct task_struct *task;
	struct task_struct *ttask;

	spin_lock_irqsave(&task_mortuary, flags);

	list_splice_init(&dead_tasks, &local_dead_tasks);
	list_splice_init(&dying_tasks, &dead_tasks);

	spin_unlock_irqrestore(&task_mortuary, flags);

	list_for_each_entry_safe(task, ttask, &local_dead_tasks, tasks) {
		list_del(&task->tasks);
		free_task(task);
	}
}

void mark_done(int cpu)
{
	int i;

	mutex_lock(&die_mutex);
	cpumask_set_cpu(cpu, marked_cpus);

	for_each_online_cpu(i) {
		if (!cpumask_test_cpu(i, marked_cpus)) {
			mutex_unlock(&die_mutex);
			return;
		}
	}
	/* All CPUs have been processed at least once,
	 * we can process the mortuary once
	 */
	process_task_mortuary();

	cpumask_clear(marked_cpus);
	mutex_unlock(&die_mutex);
}

static void free_all_tasks(void)
{
	/* make sure we don't leak task structs */
	process_task_mortuary();
	process_task_mortuary();
}

/* Take ownership of the task struct and place it on the
 * list for processing. Only after two full buffer syncs
 * does the task eventually get freed, because by then
 * we are sure we will not reference it again.
 * Can be invoked from softirq via RCU callback due to
 * call_rcu() of the task struct, hence the _irqsave.
 */
static int task_free_notify(struct notifier_block *self, unsigned long val, void *data)
{
	unsigned long flags;
	struct task_struct *task = data;
	spin_lock_irqsave(&task_mortuary, flags);
	list_add(&task->tasks, &dying_tasks);
	spin_unlock_irqrestore(&task_mortuary, flags);
	return NOTIFY_OK;
}

static int task_exit_notify(struct notifier_block *self, unsigned long val, void *data)
{
//	struct task_struct *task = data;
	sync_all_samples();
	return 0;
}

static int munmap_notify(struct notifier_block *self, unsigned long val, void *data)
{
//	struct task_struct *task = data;
	sync_all_samples();
	return 0;
}

static int module_load_notify(struct notifier_block *self, unsigned long val, void *data)
{
	return 0;
}

static struct notifier_block task_free_nb = {
	.notifier_call	= task_free_notify,
};

static struct notifier_block task_exit_nb = {
	.notifier_call	= task_exit_notify,
};

static struct notifier_block munmap_nb = {
	.notifier_call	= munmap_notify,
};

static struct notifier_block module_load_nb = {
	.notifier_call = module_load_notify,
};

struct mm_struct *take_tasks_mm(struct task_struct *task)
{
	struct mm_struct *mm = get_task_mm(task);
	if (mm)
		down_read(&mm->mmap_sem);
	return mm;
}

void release_mm(struct mm_struct *mm)
{
	if (!mm)
		return;
	up_read(&mm->mmap_sem);
	mmput(mm);
}

static inline unsigned long fast_get_dcookie(struct path *path)
{
	unsigned long cookie = 0;

	if (path->dentry->d_flags & DCACHE_COOKIE)
		return (unsigned long)path->dentry;
	get_dcookie(path, &cookie);
	return cookie;
}

unsigned long lookup_dcookie(struct mm_struct *mm, unsigned long addr, off_t *offset)
{
	unsigned long cookie = NO_COOKIE;
	struct vm_area_struct *vma;

	for (vma = find_vma(mm, addr); vma; vma = vma->vm_next) {

		if (addr < vma->vm_start || addr >= vma->vm_end)
			continue;

		if (vma->vm_file) {
			cookie = fast_get_dcookie(&vma->vm_file->f_path);
			*offset = (vma->vm_pgoff << PAGE_SHIFT) + addr -
			           vma->vm_start;
		} else {
			/* must be an anonymous map */
			*offset = addr;
		}

		break;
	}

	if (!vma)
		cookie = INVALID_COOKIE;

	return cookie;
}

int src_map_start(void)
{
	dcookie = dcookie_register();

	if (!zalloc_cpumask_var(&marked_cpus, GFP_KERNEL))
		return -ENOMEM;

	task_handoff_register(&task_free_nb);
	profile_event_register(PROFILE_TASK_EXIT, &task_exit_nb);
	profile_event_register(PROFILE_MUNMAP, &munmap_nb);
	register_module_notifier(&module_load_nb);

	return 0;
}

void src_map_stop(void)
{
	unregister_module_notifier(&module_load_nb);
	profile_event_unregister(PROFILE_MUNMAP, &munmap_nb);
	profile_event_unregister(PROFILE_TASK_EXIT, &task_exit_nb);
	task_handoff_unregister(&task_free_nb);

	free_all_tasks();
	free_cpumask_var(marked_cpus);
}

void src_map_stop_dcookie(void)
{
	if (dcookie != NULL) {
		dcookie_unregister(dcookie);
		dcookie	= NULL;
	}
}
