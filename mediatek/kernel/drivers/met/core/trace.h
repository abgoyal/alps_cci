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

#ifndef _TRACE_H_
#define _TRACE_H_

#define FMT1	",%x\n"
#define FMT2	",%x,%x\n"
#define FMT3	",%x,%x,%x\n"
#define FMT4	",%x,%x,%x,%x\n"
#define FMT5	",%x,%x,%x,%x,%x\n"
#define FMT6	",%x,%x,%x,%x,%x,%x\n"
#define FMT7	",%x,%x,%x,%x,%x,%x,%x\n"
#define FMT8	",%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT9	",%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT10	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"

#define FMT14	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT17	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT18	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT19	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT30	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT34	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT37	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT44	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT50	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT66	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"
#define FMT82	",%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n"

#define VAL1	,value[0]
#define VAL2	,value[0],value[1]
#define VAL3	,value[0],value[1],value[2]
#define VAL4	,value[0],value[1],value[2],value[3]
#define VAL5	,value[0],value[1],value[2],value[3],value[4]
#define VAL6	,value[0],value[1],value[2],value[3],value[4],value[5]
#define VAL7	,value[0],value[1],value[2],value[3],value[4],value[5],value[6]
#define VAL8	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7]
#define VAL9	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8]
#define VAL10	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]

#define VAL14	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13]
#define VAL17	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16]
#define VAL18	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17]
#define VAL19	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18]

#define VAL30	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]

#define VAL34	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33]

#define VAL37	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33],value[34],value[35],value[36]

#define VAL44	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33],value[34],value[35],value[36],value[37],value[38],value[39]\
		,value[40],value[41],value[42],value[43]
#define VAL50	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33],value[34],value[35],value[36],value[37],value[38],value[39]\
		,value[40],value[41],value[42],value[43],value[44],value[45],value[46],value[47],value[48],value[49]
#define VAL66	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33],value[34],value[35],value[36],value[37],value[38],value[39]\
		,value[40],value[41],value[42],value[43],value[44],value[45],value[46],value[47],value[48],value[49]\
		,value[50],value[51],value[52],value[53],value[54],value[55],value[56],value[57],value[58],value[59]\
		,value[60],value[61],value[62],value[63],value[64],value[65]
#define VAL82	,value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],value[8],value[9]\
                ,value[10],value[11],value[12],value[13],value[14],value[15],value[16],value[17],value[18],value[19]\
                ,value[20],value[21],value[22],value[23],value[24],value[25],value[26],value[27],value[28],value[29]\
		,value[30],value[31],value[32],value[33],value[34],value[35],value[36],value[37],value[38],value[39]\
		,value[40],value[41],value[42],value[43],value[44],value[45],value[46],value[47],value[48],value[49]\
		,value[50],value[51],value[52],value[53],value[54],value[55],value[56],value[57],value[58],value[59]\
		,value[60],value[61],value[62],value[63],value[64],value[65],value[66],value[67],value[68],value[69]\
		,value[70],value[71],value[72],value[73],value[74],value[75],value[76],value[77],value[78],value[79]\
		,value[80],value[81]


void mp_cp(unsigned long long timestamp,
	   struct task_struct *task,
	   unsigned long program_counter,
	   unsigned long dcookie,
	   unsigned long offset,
	   unsigned char cnt, unsigned int *value);

void mp_cp2(unsigned long long timestamp,
	   struct task_struct *task,
	   unsigned long program_counter,
	   unsigned long dcookie,
	   unsigned long offset,
	   unsigned char cnt, unsigned int *value);

void cpu_frequency(unsigned int frequency, unsigned int cpu_id);
extern void (*mp_cp_ptr)(unsigned long long timestamp,
	       struct task_struct *task,
	       unsigned long program_counter,
	       unsigned long dcookie,
	       unsigned long offset,
	       unsigned char cnt, unsigned int *value);

#endif // _TRACE_H_
