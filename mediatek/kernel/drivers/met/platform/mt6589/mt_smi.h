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

#ifndef __MT_SMI_H__
#define __MT_SMI_H__


#define SMI_LARB_NUMBER 5
#define SMI_COMM_NUMBER 1

#define SMI_REQ_OK           (0)
#define SMI_ERR_WRONG_REQ    (-1)
#define SMI_ERR_OVERRUN      (-2)

#define SMI_LARB_MON_ENA(i) (i + 0x400)
#define SMI_LARB_MON_CLR(i) (i + 0x404)
#define SMI_LARB_MON_PORT(i) (i + 0x408)
#define SMI_LARB_MON_TYPE(i) (i + 0x40C)
#define SMI_LARB_MON_CON(i) (i + 0x410)

#define SMI_LARB_MON_ACT_CNT(i) (i + 0x420)
#define SMI_LARB_MON_REQ_CNT(i) (i + 0x424)
#define SMI_LARB_MON_IDL_CNT(i) (i + 0x428)
#define SMI_LARB_MON_BEA_CNT(i) (i + 0x42C)
#define SMI_LARB_MON_BYT_CNT(i) (i + 0x430)
#define SMI_LARB_MON_CP_CNT(i) (i + 0x434)
#define SMI_LARB_MON_DP_CNT(i) (i + 0x438)
#define SMI_LARB_MON_CDP_MAX(i) (i + 0x43C)
#define SMI_LARB_MON_COS_MAX(i) (i + 0x440)
#define SMI_LARB_MON_BUS_REQ0(i) (i + 0x450)

#define SMI_LARB_MON_BUS_REQ1(i) (i + 0x454)
#define SMI_LARB_MON_WDT_CNT(i) (i + 0x460)
#define SMI_LARB_MON_RDT_CNT(i) (i + 0x464)
#define SMI_LARB_MON_OST_CNT(i) (i + 0x468)

#define SMI_COMM_MON_ENA(i) (i + 0x1A0)
#define SMI_COMM_MON_CLR(i) (i + 0x1A4)
//#define SMI_COMM_MON_PORT(i) (i + 0x408)
#define SMI_COMM_MON_TYPE(i) (i + 0x1AC)
#define SMI_COMM_MON_CON(i) (i + 0x1B0)
#define SMI_COMM_MON_ACT_CNT(i) (i + 0x1C0)
#define SMI_COMM_MON_REQ_CNT(i) (i + 0x1C4)
#define SMI_COMM_MON_IDL_CNT(i) (i + 0x1C8)
#define SMI_COMM_MON_BEA_CNT(i) (i + 0x1CC)
#define SMI_COMM_MON_BYT_CNT(i) (i + 0x1D0)
#define SMI_COMM_MON_CP_CNT(i) (i + 0x1D4)
#define SMI_COMM_MON_DP_CNT(i) (i + 0x1D8)
#define SMI_COMM_MON_CDP_MAX(i) (i + 0x1DC)
#define SMI_COMM_MON_COS_MAX(i) (i + 0x1E0)
//#define SMI_COMM_MON_BUS_REQ0(i) (i + 0x450)



typedef struct {
    unsigned long bIdleSelection : 1; // 0 : idle count increase when no request, and outstanding request is less than , 1 : idle count increase when there is no request and read/write data transfer.
    unsigned long uIdleOutStandingThresh : 3;
    unsigned long bDPSelection : 1; // 0 : data phase incresae 1 when any outstanding transaction waits for data transfer. 1 : data phase increase N when N out standing transaction are waiting.
    unsigned long bMaxPhaseSelection : 1;// 0 : Command pahse , 1 : Data phase.
    unsigned long bRequestSelection : 2;// 00:All, 01:ultra high, 10:pre-ultra high, 11:normal.
    unsigned long bStarvationEn : 1; // 0 : disable , 1 : Enable
    unsigned long uStarvationTime : 8;
    unsigned long u2Reserved : 12; //Reserved
}SMIBMCfg_Ext;


void SMI_Init(void);
void SMI_SetSMIBMCfg(unsigned long larbno, unsigned long portno, unsigned long desttype, unsigned long rwtype);
void SMI_SetMonitorControl (SMIBMCfg_Ext *cfg_ex);
void SMI_Enable(unsigned long larbno, unsigned long bustype);
void SMI_Disable(unsigned long larbno);
void SMI_Pause(int larbno);
void SMI_Clear(int larbno);
void SMI_PowerOn(void);
void SMI_PowerOff(void);

int SMI_GetPortNo(int larbno);
int SMI_GetActiveCnt(int larbno);
int SMI_GetRequestCnt(int larbno);
int SMI_GetIdleCnt(int larbno);
int SMI_GetBeatCnt(int larbno);
int SMI_GetByteCnt(int larbno);
int SMI_GetCPCnt(int larbno);
int SMI_GetDPCnt(int larbno);
int SMI_GetCDP_MAX(int larbno);
int SMI_GetCOS_MAX(int larbno);
int SMI_GetBUS_REQ0(int larbno);
int SMI_GetBUS_REQ1(int larbno);
int SMI_GetWDTCnt(int larbno);
int SMI_GetRDTCnt(int larbno);
int SMI_GetOSTCnt(int larbno);

//common
void SMI_Comm_Init(void);
void SMI_SetCommBMCfg(unsigned long commonno, unsigned long portno, unsigned long desttype, unsigned long rwtype);
void SMI_Comm_Enable(unsigned long commonno);
void SMI_Comm_Disable(unsigned long commonno);
void SMI_Pause(int commonno);
void SMI_Comm_Clear(int commonno);

int SMI_Comm_GetPortNo(int commonno);
int SMI_Comm_GetActiveCnt(int commonno);
int SMI_Comm_GetRequestCnt(int commonno);
int SMI_Comm_GetIdleCnt(int commonno);
int SMI_Comm_GetBeatCnt(int commonno);
int SMI_Comm_GetByteCnt(int commonno);
int SMI_Comm_GetCPCnt(int commonno);
int SMI_Comm_GetDPCnt(int commonno);
int SMI_Comm_GetCDP_MAX(int commonno);
int SMI_Comm_GetCOS_MAX(int commonno);

#endif  /* !__MT_SMI_H__ */