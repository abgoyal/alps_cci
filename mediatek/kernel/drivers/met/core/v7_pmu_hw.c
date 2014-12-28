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

#include <asm/system.h>
#include <linux/smp.h>
#include "v7_pmu.h"
#include "v7_pmu_hw.h"

enum ARM_TYPE armv7_get_ic(void)
{
	unsigned int value;
	// Read Main ID Register
	asm volatile("mrc p15, 0, %0, c0, c0, 0" : "=r" (value));

	value = (value & 0xffff) >> 4; // primary part number
	return value;
}

static inline void armv7_pmu_counter_select(unsigned int idx)
{
	asm volatile("mcr p15, 0, %0, c9, c12, 5" : : "r" (idx));
	isb();
}

static inline void armv7_pmu_type_select(unsigned int idx, unsigned int type)
{
	armv7_pmu_counter_select(idx);
	asm volatile("mcr p15, 0, %0, c9, c13, 1" : : "r" (type));
}

static inline unsigned int armv7_pmu_read_count(unsigned int idx)
{
	unsigned int value;

	if (idx == 31) {
		asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r" (value));
	} else {
		armv7_pmu_counter_select(idx);
		asm volatile("mrc p15, 0, %0, c9, c13, 2" : "=r" (value));
	}
	return value;
}

static inline void armv7_pmu_write_count(int idx, u32 value)
{
	if (idx == 31) {
		asm volatile("mcr p15, 0, %0, c9, c13, 0" : : "r" (value));
	} else {
		armv7_pmu_counter_select(idx);
		asm volatile("mcr p15, 0, %0, c9, c13, 2" : : "r" (value));
	}
}

static inline void armv7_pmu_enable_count(unsigned int idx)
{
	asm volatile("mcr p15, 0, %0, c9, c12, 1" : : "r" (1 << idx));
}

static inline void armv7_pmu_disable_count(unsigned int idx)
{
	asm volatile("mcr p15, 0, %0, c9, c12, 2" : : "r" (1 << idx));
}

static inline void armv7_pmu_enable_intr(unsigned int idx)
{
	asm volatile("mcr p15, 0, %0, c9, c14, 1" : : "r" (1 << idx));
}

static inline void armv7_pmu_disable_intr(unsigned int idx)
{
	asm volatile("mcr p15, 0, %0, c9, c14, 2" : : "r" (1 << idx));
}

static inline unsigned int armv7_pmu_overflow(void)
{
	unsigned int val;
	asm volatile("mrc p15, 0, %0, c9, c12, 3" : "=r" (val)); // read
	asm volatile("mcr p15, 0, %0, c9, c12, 3" : : "r" (val)); // clear
	return val;
}

static inline unsigned int armv7_pmu_control_read(void)
{
	u32 val;
	asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r" (val));
	return val;
}

static inline void armv7_pmu_control_write(unsigned int val)
{
	val &= ARMV7_PMCR_MASK;
	isb();
	asm volatile("mcr p15, 0, %0, c9, c12, 0" : : "r" (val));
}

int armv7_pmu_hw_get_counters(void)
{
	int count = armv7_pmu_control_read();
	// N, bits[15:11]
	count = ((count >> 11) & 0x1f);
	return count;
}

static void armv7_pmu_hw_reset_all(int generic_counters)
{
	int i;
	armv7_pmu_control_write(ARMV7_PMCR_C | ARMV7_PMCR_P);
	// generic counter
	for (i=0; i<generic_counters; i++) {
		armv7_pmu_disable_intr(i);
		armv7_pmu_disable_count(i);
	}
	// cycle counter
	armv7_pmu_disable_intr(31);
	armv7_pmu_disable_count(31);
	armv7_pmu_overflow(); // clear overflow
}

void armv7_pmu_hw_start(struct met_pmu *pmu, int count)
{
	int i;
	int generic = count-1;

	armv7_pmu_hw_reset_all(generic);
	for (i=0; i<generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			armv7_pmu_type_select(i, pmu[i].event);
			armv7_pmu_enable_count(i);
		}
	}
	if (pmu[count-1].mode == MODE_POLLING) { // cycle counter
		armv7_pmu_enable_count(31);
	}
	armv7_pmu_control_write(ARMV7_PMCR_E);
}

void armv7_pmu_hw_stop(int count)
{
	int generic = count-1;
	armv7_pmu_hw_reset_all(generic);
}

unsigned int armv7_pmu_hw_polling(struct met_pmu *pmu, int count, unsigned int *pmu_value)
{
	int i, cnt=0;
	int generic = count-1;

	for (i=0; i<generic; i++) {
		if (pmu[i].mode == MODE_POLLING) {
			pmu_value[cnt] = armv7_pmu_read_count(i);
			cnt++;
		}
	}
	if (pmu[count-1].mode == MODE_POLLING) {
		pmu_value[cnt] = armv7_pmu_read_count(31);
		cnt++;
	}
	armv7_pmu_control_write(ARMV7_PMCR_C | ARMV7_PMCR_P | ARMV7_PMCR_E);

	return cnt;
}
