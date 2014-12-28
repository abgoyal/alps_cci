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
#include <linux/device.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/hrtimer.h>

#include "core/met_drv.h"
#include "core/trace.h"

#include "thermal.h"
#include "plf_trace.h"

extern struct metdevice met_thermal;
struct delayed_work dwork;

static int do_thermal(void)
{
	static int do_thermal = -1;

	if (do_thermal != -1) {
		return do_thermal;
	}

	if (met_thermal.mode == 0) {
		do_thermal = 0;
	} else {
        do_thermal = met_thermal.mode;
    }
	return do_thermal;
}

#if NO_MTK_THERMAL_GET_TEMP == 0
static unsigned int get_thermal(unsigned int *value)
{
	int i;
	int j = -1;

	for (i=0; i<MTK_THERMAL_SENSOR_COUNT;i++) {
		value[++j]=mtk_thermal_get_temp(i);
	}
	return j+1;
}
#endif

static void wq_get_thermal(struct work_struct *work)
{
	unsigned char count=0;
	unsigned int thermal_value[MTK_THERMAL_SENSOR_COUNT];  //Note here

	int cpu;
	unsigned long long stamp;
	//return;
	cpu = smp_processor_id();
	if (do_thermal()) {
		stamp = cpu_clock(cpu);
#if NO_MTK_THERMAL_GET_TEMP == 0
		count = get_thermal(thermal_value);
#endif
		if (count) {
			ms_th(stamp, count, thermal_value);
		}
	}

}

static void thermal_start(void)
{
	INIT_DELAYED_WORK(&dwork, wq_get_thermal);
	return;
}

static void thermal_stop(void)
{
	cancel_delayed_work_sync(&dwork);
	return;
}

static void thermal_polling(unsigned long long stamp, int cpu)
{
	schedule_delayed_work(&dwork, 0);
}

static char header[] =
"met-info [000] 0.0: ms_ud_sys_header: ms_th,timestamp,cpu,abb,pmic,battery,md1,md2,wifi,d,d,d,d,d,d,d\n";
static char help[] = "  --thermal                             monitor thermal\n";

static int thermal_print_help(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, help);
}

static int thermal_print_header(char *buf, int len)
{
	return snprintf(buf, PAGE_SIZE, header);
}

/*
static int thermal_process_argument(const char *arg, int len)
{
	printk("Thermal Argument(l=%d):%s\n", len, arg);
	return 0;
}
*/

struct metdevice met_thermal = {
	.name = "thermal",
	.owner = THIS_MODULE,
	.type = MET_TYPE_BUS,
	.cpu_related = 0,
	.start = thermal_start,
	.stop = thermal_stop,
	.polling_interval = 1000,//1s
	.timed_polling = thermal_polling,
	.tagged_polling = thermal_polling,
	.print_help = thermal_print_help,
	.print_header = thermal_print_header,
//	.process_argument = thermal_process_argument
};
