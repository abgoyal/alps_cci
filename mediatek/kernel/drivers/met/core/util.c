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

#include "util.h"
#include <linux/fs.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>

#ifdef FILELOG

static char tmp[1000] = { 0 };

/*TODO*/
/**
 * open file
 * @param name path to open
 * @return file pointer
 */
struct file *open_file(const char *name)
{
	struct file *fp = NULL;

	fp = filp_open(name, O_WRONLY | O_APPEND /*| O_TRUNC*/ | O_CREAT, 0644);
	if (unlikely(fp == NULL)) {
		printk("can not open result file");
		return NULL;
	}
	return fp;
}

/**
 * write to file
 * @param fp file pointer
 * @param format format string
 * @param ... variable-length subsequent arguments
 */
void write_file(struct file *fp, const char *format, ...)
{
	va_list va;
	mm_segment_t fs = get_fs();

	va_start(va, format);
	vsnprintf(tmp, sizeof(tmp), format, va);
	set_fs(KERNEL_DS);
	vfs_write(fp, tmp, strlen(tmp), &(fp->f_pos));
	set_fs(fs);
	va_end(va);
}

/**
 * close file
 * @param fp file pointer
 * @return exit code
 */
int close_file(struct file *fp)
{
	if (likely(fp != NULL)) {
		filp_close(fp, NULL);
		fp = NULL;
		return 0;
	} else {
		printk("cannot close file pointer:%p\n", fp);
		return -1;
	}
}

void filelog(char *str)
{
	struct file *fp;

	fp = open_file("/data/met.log");
	if (fp != NULL) {
		write_file(fp, "%s", str);
		close_file(fp);
	}
}

#endif //FILELOG
