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

#ifndef __MCI_H__
#define __MCI_H__

/*
 * Define hardware registers.
 */
#define MCI_CON_OVRD        (MCI_BASE + 0x0000)
#define MCI_SFETCH_CON      (MCI_BASE + 0x0004)
#define MCI_SEC_CON         (MCI_BASE + 0x0008)
#define MCI_ERR_REG         (MCI_BASE + 0x0010)
#define MCI_PMCR            (MCI_BASE + 0x0014)
#define MCI_SCR_S0          (MCI_BASE + 0x0100)
#define MCI_SOR_S0          (MCI_BASE + 0x0104)
#define MCI_RQOS_OVRD_S0    (MCI_BASE + 0x0108)
#define MCI_WQOS_OVRD_S0    (MCI_BASE + 0x010C)
#define MCI_SCR_S1          (MCI_BASE + 0x0200)
#define MCI_SOR_S1          (MCI_BASE + 0x0204)
#define MCI_RQOS_OVRD_S1    (MCI_BASE + 0x0208)
#define MCI_WQOS_OVRD_S1    (MCI_BASE + 0x020C)
#define MCI_CCR             (MCI_BASE + 0x0300)
#define MCI_CCR_CON         (MCI_BASE + 0x0304)
#define MCI_CCR_OVFL        (MCI_BASE + 0x0308)
#define MCI_EVENT0_SEL      (MCI_BASE + 0x0400)
#define MCI_EVENT0_CNT      (MCI_BASE + 0x0404)
#define MCI_EVENT0_CON      (MCI_BASE + 0x0408)
#define MCI_EVENT0_OVFL     (MCI_BASE + 0x040C)
#define MCI_EVENT1_SEL      (MCI_BASE + 0x0500)
#define MCI_EVENT1_CNT      (MCI_BASE + 0x0504)
#define MCI_EVENT1_CON      (MCI_BASE + 0x0508)
#define MCI_EVENT1_OVFL     (MCI_BASE + 0x050C)



/*
 * Define constants.
 */


/*
 * Define function prototypes.
 */
   #if defined(CONFIG_MTK_MCI)
   void mci_snoop_sleep(void);
   void mci_snoop_restore(void);
   void mci_restore(void);
   void mci_sleep(void);
   #else
   #define mci_snoop_sleep() do {} while(0)
   #define mci_snoop_restore() do {} while(0)
   #define mci_sleep() do {} while(0)
   #define mci_restore() do {} while(0)
   #endif//CONFIG_MTK_MCI
#endif  /*!__MCI_H__ */
