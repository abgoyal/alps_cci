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

#ifndef _V7_PMU_NAME_H_
#define _V7_PMU_NAME_H_

struct pmu_desc {
	unsigned int event;
	char name[32];
};

/* Cortex-A7 */
struct pmu_desc a7_pmu_desc[] = {
	{ 0x00, "SW_INCR" },
	{ 0x01, "L1I_CACHE_REFILL" },
	{ 0x02, "L1I_TLB_REFILL" },
	{ 0x03, "L1D_CACHE_REFILL" },
	{ 0x04, "L1D_CACHE" },
	{ 0x05, "L1D_TLB_REFILL" },
	{ 0x06, "LD_RETIRED" },
	{ 0x07, "ST_RETIRED" },
	{ 0x08, "INST_RETIRED" },
	{ 0x09, "EXC_TAKEN" },
	{ 0x0A, "EXC_RETURN" },
	{ 0x0B, "CID_WRITE_RETIRED" },
	{ 0x0C, "PC_WRITE_RETIRED" },
	{ 0x0D, "BR_IMMED_RETIRED" },
	{ 0x0E, "BR_RETURN_RETIRED" },
	{ 0x0F, "UNALIGNED_LDST_RETIRED" },
	{ 0x10, "BR_MIS_PRED" },
	{ 0x12, "BR_PRED" },
	{ 0x13, "MEM_ACCESS" },
	{ 0x14, "L1I_CACHE" },
	{ 0x15, "L1D_CACHE_WB" },
	{ 0x16, "L2D_CACHE" },
	{ 0x17, "L2D_CACHE_REFILL" },
	{ 0x18, "L2D_CACHE_WB" },
	{ 0x19, "BUS_ACCESS" },
	{ 0x1D, "BUS_CYCLES" },
	{ 0x60, "BUS_READ_ACCESS" },
	{ 0x61, "BUS_WRITE_ACCESS" },
	{ 0x86, "IRQ_EXC_TAKEN" },
	{ 0x87, "FIQ_EXC_TAKEN" },
	{ 0xC0, "EXT_MEM_REQ" },
	{ 0xC1, "NO_CACHE_EXT_MEM_REQ" },
	{ 0xC2, "PREFETCH_LINEFILL" },
	{ 0xC3, "PREFETCH_LINEFILL_DROPPED" },
	{ 0xC4, "ENT_READ_ALLOC_MODE" },
	{ 0xC5, "READ_ALLOC_MODE" },
	{ 0xC7, "ETM_EXT_OUT0" },
	{ 0xC8, "ETM_EXT_OUT1" },
	{ 0xC9, "DATA_WRITE_STALL" },
	{ 0xCA, "DATA_READ_SNOOP_CLUSTER" }
};

/* Cortex-A9 */
struct pmu_desc a9_pmu_desc[] = {
	{ 0x00, "SW_INCR" },
	{ 0x01, "L1I_CACHE_REFILL" },
	{ 0x02, "L1I_TLB_REFILL" },
	{ 0x03, "L1D_CACHE_REFILL" },
	{ 0x04, "L1D_CACHE" },
	{ 0x05, "L1D_TLB_REFILL" },
	{ 0x06, "LD_RETIRED" },
	{ 0x07, "ST_RETIRED" },
	{ 0x09, "EXC_TAKEN" },
	{ 0x0A, "EXC_RETURN" },
	{ 0x0B, "CID_WRITE_RETIRED" },
	{ 0x0C, "PC_WRITE_RETIRED" },
	{ 0x0D, "BR_IMMED_RETIRED" },
	{ 0x0F, "UNALIGNED_LDST_RETIRED" },
	{ 0x10, "BR_MIS_PRED" },
	{ 0x12, "BR_PRED" },
	{ 0x40, "JAVA_BC_EXEC" },
	{ 0x41, "SW_JAVA_BC_EXEC" },
	{ 0x42, "JAZELLE_BB_EXEC" },
	{ 0x50, "CO_LF_MISS" },
	{ 0x51, "CO_LF_HIT" },
	{ 0x60, "ICACHE_DEP_STALL" },
	{ 0x61, "DCACHE_DEP_STALL" },
	{ 0x62, "M_TLB_STALL" },
	{ 0x63, "STREX_PASSED" },
	{ 0x64, "STREX_FAILED" },
	{ 0x65, "DATA_EVICT" },
	{ 0x66, "ISSUE_NO_DISP" },
	{ 0x67, "ISSUE_EMPTY" },
	{ 0x68, "INS_RENAME" },
	{ 0x6E, "PRED_FN_RET" },
	{ 0x70, "MAIN_EXEC_INST" },
	{ 0x71, "SEC_EXEC_INST" },
	{ 0x72, "LOAD_STORE_INST" },
	{ 0x73, "FLOAT_INST_RR" },
	{ 0x74, "NEON_INST_RR" },
	{ 0x80, "STALL_PLD" },
	{ 0x81, "STALL_WRITE" },
	{ 0x82, "STALL_INST_M_TLB_MISS" },
	{ 0x83, "STALL_DATA_M_TLB_MISS" },
	{ 0x84, "STALL_INST_U_TLB" },
	{ 0x85, "STALL_DATA_U_TLB" },
	{ 0x86, "STALL_DMB" },
	{ 0x8A, "INT_CLK_EN" },
	{ 0x8B, "DATA_E_CLK_EN" },
	{ 0x90, "ISB_INST" },
	{ 0x91, "DSB_INST" },
	{ 0x92, "INS_DMB" },
	{ 0x93, "EXT_IRQ" },
	{ 0xA0, "PLE_CACHE_REQ_COMP" },
	{ 0xA1, "PLE_CACHE_REQ_SKP" },
	{ 0xA2, "PLE_FIFO_FLUSH" },
	{ 0xA3, "PLE_REQ_COMP" },
	{ 0xA4, "PLE_FIFO_OF" },
	{ 0xA5, "PLE_REQ_PRG" }
};

#define A7_PMU_DESC_COUNT (sizeof(a7_pmu_desc) / sizeof(struct pmu_desc))
#define A9_PMU_DESC_COUNT (sizeof(a9_pmu_desc) / sizeof(struct pmu_desc))

#endif // _V7_PMU_NAME_H_
