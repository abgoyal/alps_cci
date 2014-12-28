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

#ifndef _MCI_NAME_H_
#define _MCI_NAME_H_

struct mci_desc {
	unsigned int event;
	char name[32];
};

struct mci_desc mci_desc[] = {
	{ 0x00, "Si0 AR transaction counts" },
	{ 0x01, "Si0 AW transaction counts" },
	{ 0x02, "AC_R transaction counts" },
	{ 0x03, "AC_W transaction counts" },
	{ 0x04, "AC_R snoop hit" },
	{ 0x05, "AC_W snoop hit" },
	{ 0x06, "Si0 AR input queue full" },
	{ 0x07, "Si0 AW input queue full" },
	{ 0x08, "Si0 AR order table full" },
	{ 0x09, "Si0 AW order table full" },
	{ 0x0a, "Reserved" },
	{ 0x0b, "Si0 AR order table stall" },
	{ 0x0c, "Si0 AW order table stall" },
	{ 0x0d, "Si1 AR transaction counts" },
	{ 0x0e, "Si1 AW transaction counts" },
	{ 0x0f, "Si1 AR input queue full" },
	{ 0x10, "Si1 AW input queue full" },
	{ 0x11, "Si1 AR order table full" },
	{ 0x12, "Si1 AW order table full" },
	{ 0x13, "Reserved" },
	{ 0x14, "Si1 AR order table full" },
	{ 0x15, "Si1 AW order table full" },
	{ 0x16, "Si1 ar_tracker full" },
	{ 0x17, "Si0 aw_tracker full" },
	{ 0x18, "Si1 tracker contention" }
};

#define MCI_DESC_COUNT (sizeof(mci_desc) / sizeof(struct mci_desc))

#endif // _MCI_NAME_H_