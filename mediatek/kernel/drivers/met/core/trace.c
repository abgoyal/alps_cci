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
#include <linux/module.h>
#include <linux/sched.h>
#include "trace.h"

#define MP_CP_FMT	"%5lu.%06lu,%d,0x%lx,0x%lx,0x%lx"
#define MP_CP_VAL	(unsigned long)(timestamp), nano_rem/1000, \
	task->pid, program_counter, dcookie, offset
void mp_cp(unsigned long long timestamp,
	       struct task_struct *task,
	       unsigned long program_counter,
	       unsigned long dcookie,
	       unsigned long offset,
	       unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 1: trace_printk(MP_CP_FMT FMT1, MP_CP_VAL VAL1); break;
	case 2: trace_printk(MP_CP_FMT FMT2, MP_CP_VAL VAL2); break;
	case 3: trace_printk(MP_CP_FMT FMT3, MP_CP_VAL VAL3); break;
	case 4: trace_printk(MP_CP_FMT FMT4, MP_CP_VAL VAL4); break;
	case 5: trace_printk(MP_CP_FMT FMT5, MP_CP_VAL VAL5); break;
	case 6: trace_printk(MP_CP_FMT FMT6, MP_CP_VAL VAL6); break;
	case 7: trace_printk(MP_CP_FMT FMT7, MP_CP_VAL VAL7); break;
	case 8: trace_printk(MP_CP_FMT FMT8, MP_CP_VAL VAL8); break;
	case 9: trace_printk(MP_CP_FMT FMT9, MP_CP_VAL VAL9); break;
	}
}

#define MP_CP_FMT2	"%5lu.%06lu,%d,%s,0x%lx,0x%lx,0x%lx"
#define MP_CP_VAL2	(unsigned long)(timestamp), nano_rem/1000, \
	task->pid, task->comm, program_counter, dcookie, offset
void mp_cp2(unsigned long long timestamp,
	       struct task_struct *task,
	       unsigned long program_counter,
	       unsigned long dcookie,
	       unsigned long offset,
	       unsigned char cnt, unsigned int *value)
{
	unsigned long nano_rem = do_div(timestamp, 1000000000);
	switch (cnt) {
	case 1: trace_printk(MP_CP_FMT2 FMT1, MP_CP_VAL2 VAL1); break;
	case 2: trace_printk(MP_CP_FMT2 FMT2, MP_CP_VAL2 VAL2); break;
	case 3: trace_printk(MP_CP_FMT2 FMT3, MP_CP_VAL2 VAL3); break;
	case 4: trace_printk(MP_CP_FMT2 FMT4, MP_CP_VAL2 VAL4); break;
	case 5: trace_printk(MP_CP_FMT2 FMT5, MP_CP_VAL2 VAL5); break;
	case 6: trace_printk(MP_CP_FMT2 FMT6, MP_CP_VAL2 VAL6); break;
	case 7: trace_printk(MP_CP_FMT2 FMT7, MP_CP_VAL2 VAL7); break;
	case 8: trace_printk(MP_CP_FMT2 FMT8, MP_CP_VAL2 VAL8); break;
	case 9: trace_printk(MP_CP_FMT2 FMT9, MP_CP_VAL2 VAL9); break;
	}
}

void cpu_frequency(unsigned int frequency, unsigned int cpu_id)
{
	trace_printk("state=%d cpu_id=%d\n", frequency, cpu_id);
}

void (*mp_cp_ptr)(unsigned long long timestamp,
	       struct task_struct *task,
	       unsigned long program_counter,
	       unsigned long dcookie,
	       unsigned long offset,
	       unsigned char cnt, unsigned int *value) = &mp_cp;
