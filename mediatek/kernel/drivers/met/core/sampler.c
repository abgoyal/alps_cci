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


#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/module.h>

#include "buffer.h"
#include "sampler.h"
#include "met_struct.h"
#include "v7_pmu.h"
#include "util.h"

#include "trace.h"
#include "met_drv.h"

#ifdef CONFIG_CPU_FREQ
#include "power.h"

extern volatile int do_dvfs;
#endif

extern struct list_head met_list;

static volatile int start = 0;

static void wq_sync_buffer(struct work_struct *work)
{
	int cpu;
	struct delayed_work *dw = container_of(work, struct delayed_work, work);
	struct met_cpu_struct *met_cpu_ptr = container_of(dw, struct met_cpu_struct, dwork);

	cpu = smp_processor_id();
	if (met_cpu_ptr->cpu != cpu) {
		// panic("ERROR");
		return;
	}

	sync_samples(cpu);
	/* don't re-add the work if we're shutting down */
	if (met_cpu_ptr->work_enabled) {
		schedule_delayed_work(dw, DEFAULT_TIMER_EXPIRE);
	}
}

static enum hrtimer_restart met_hrtimer_notify(struct hrtimer *hrtimer)
{
	int cpu;
	int *count;
	unsigned long long stamp;
	struct met_cpu_struct *met_cpu_ptr = container_of(hrtimer, struct met_cpu_struct, hrtimer);
	struct metdevice *c;

	cpu = smp_processor_id();
	if (met_cpu_ptr->cpu != cpu) {
		// panic("ERROR2");
		return HRTIMER_NORESTART;
	}

	add_sample(get_irq_regs(), cpu);

	list_for_each_entry(c, &met_list, list) {

		if ((c->mode==0)||(c->timed_polling == NULL)) {
			continue;
		}

		count = per_cpu_ptr(c->polling_count, cpu);
		if ((*count) > 0) {
			(*count)--;
			continue;
		}

		*(count) = c->polling_count_reload;

		stamp = cpu_clock(cpu);

		if (c->cpu_related == 0) {
			if (cpu == 0) {
				c->timed_polling(stamp, 0);
			}
		}
		else {
			c->timed_polling(stamp, cpu);
		}
	}

	if (met_cpu_ptr->work_enabled) {
		hrtimer_forward_now(hrtimer, ns_to_ktime(DEFAULT_HRTIMER_EXPIRE));
		return HRTIMER_RESTART;
	} else {
		return HRTIMER_NORESTART;
	}
}

static void __met_hrtimer_start(void *unused)
{
	struct met_cpu_struct *met_cpu_ptr;
	struct hrtimer *hrtimer;
	struct delayed_work *dw;

	met_cpu_ptr = &__get_cpu_var(met_cpu);
	hrtimer = &met_cpu_ptr->hrtimer;
	dw = &met_cpu_ptr->dwork;

	hrtimer_init(hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer->function = met_hrtimer_notify;
	v7_pmu_start();

	if(DEFAULT_HRTIMER_EXPIRE) {
		met_cpu_ptr->work_enabled = 1;
		schedule_delayed_work_on(smp_processor_id(), dw, DEFAULT_TIMER_EXPIRE);
		hrtimer_start(hrtimer, ns_to_ktime(DEFAULT_HRTIMER_EXPIRE), HRTIMER_MODE_REL_PINNED);
	}
}

static void __met_hrtimer_stop(void *unused)
{
	struct met_cpu_struct *met_cpu_ptr;
	struct hrtimer *hrtimer;
	struct delayed_work *dw;

	met_cpu_ptr = &__get_cpu_var(met_cpu);
	hrtimer = &met_cpu_ptr->hrtimer;
	dw = &met_cpu_ptr->dwork;

	met_cpu_ptr->work_enabled = 0;
	hrtimer_cancel(hrtimer);
//	cancel_delayed_work_sync(dw);
	v7_pmu_stop();
}

static int __cpuinit met_pmu_cpu_notify(struct notifier_block *self,
					 unsigned long action, void *hcpu)
{
	struct met_cpu_struct *met_cpu_ptr;
	struct delayed_work *dw;
	long cpu = (long) hcpu;

	if (start == 0) {
		return NOTIFY_OK;
	}

	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		smp_call_function_single(cpu, __met_hrtimer_start, NULL, 1);
#ifdef CONFIG_CPU_FREQ
		if (do_dvfs != 0) {
			force_power_log(cpu);
		}
#endif
		break;

	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		smp_call_function_single(cpu, __met_hrtimer_stop, NULL, 1);

		met_cpu_ptr = &per_cpu(met_cpu, cpu);
		dw = &met_cpu_ptr->dwork;
		cancel_delayed_work_sync(dw);

		sync_samples(cpu);
		break;

	case CPU_DOWN_FAILED:
	case CPU_DOWN_FAILED_FROZEN:
		smp_call_function_single(cpu, __met_hrtimer_start, NULL, 1);
		break;

	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
#ifdef CONFIG_CPU_FREQ
		if (do_dvfs != 0) {
			force_power_log(cpu);
		}
#endif
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block __refdata met_pmu_cpu_notifier = {
	.notifier_call = met_pmu_cpu_notify,
};

int sampler_start(void)
{
	int ret, cpu;
	struct met_cpu_struct *met_cpu_ptr;
	struct metdevice *c;

	for_each_possible_cpu(cpu) {
		met_cpu_ptr = &per_cpu(met_cpu, cpu);
		met_cpu_ptr->work_enabled = 0;
		hrtimer_init(&met_cpu_ptr->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		met_cpu_ptr->hrtimer.function = met_hrtimer_notify;
		INIT_DELAYED_WORK(&met_cpu_ptr->dwork, wq_sync_buffer);
	}

	start = 0;
	ret = register_hotcpu_notifier(&met_pmu_cpu_notifier);

	list_for_each_entry(c, &met_list, list) {

		if (try_module_get(c->owner) == 0)
			continue;

		if ((c->mode)&&(c->start))
			c->start();
	}

	get_online_cpus();
	start = 1;
	on_each_cpu(__met_hrtimer_start, NULL, 1);
	put_online_cpus();


	return ret;
}

void sampler_stop(void)
{
	int cpu;
	struct met_cpu_struct *met_cpu_ptr;
	struct metdevice *c;
	struct delayed_work *dw;

	get_online_cpus();

	on_each_cpu(__met_hrtimer_stop, NULL, 1);
//	for_each_online_cpu(cpu) {
	for_each_possible_cpu(cpu) {  //Just for case
		met_cpu_ptr = &per_cpu(met_cpu, cpu);
		dw = &met_cpu_ptr->dwork;
		cancel_delayed_work_sync(dw);
		sync_samples(cpu);
	}

	start = 0;
	put_online_cpus();

	unregister_hotcpu_notifier(&met_pmu_cpu_notifier);

	list_for_each_entry(c, &met_list, list) {
		if ((c->mode)&&(c->stop))
			c->stop();

		module_put(c->owner);
	}

}
