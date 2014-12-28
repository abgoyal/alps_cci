/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef MTK_USB_CUSTOM_H
#define MTK_USB_CUSTOM_H

/* Force full-speed is not guaranteed with adb function, use it with care! */
//#define USB_FORCE_FULL_SPEED

/* Force full-speed When enter META/ATE_FACTORY mode*/
//#define USB_META_FULL_SPEED

/* Default enable CDC_ACM for normal boot */
//#define USB_CDC_ACM_NORMAL_BOOT

#define USB_MS_PRODUCT_ID           0x0001
#define USB_MS_ADB_PRODUCT_ID       0x0c03
#define USB_RNDIS_PRODUCT_ID        0x0003
#define USB_RNDIS_ADB_PRODUCT_ID    0x0004
#define USB_MS_ADB_ACM_PRODUCT_ID   0x0005
#define USB_ACM_PRODUCT_ID          0x0006
#define USB_MS_ACM_PRODUCT_ID       0x200F
#define USB_RNDIS_ADB_ACM_PRODUCT_ID 0x2010

#define VENDOR_ID      0x0bb4                /* USB vendor id  */
#define PRODUCT_ID     USB_MS_PRODUCT_ID     /* USB default product id */

/* for META/ATE_FACTORY mode */
#define SINGLE_ACM_VENDOR_ID  0x0E8D
#define SINGLE_ACM_PRODUCT_ID 0x2007

#define MANUFACTURER_STRING "MediaTek"
#define PRODUCT_STRING      "MT65xx Android Phone"

#define USB_ETH_VENDORID     0
#define USB_ETH_VENDORDESCR  "MediaTek"

#define USB_MS_VENDOR        "MediaTek"
#define USB_MS_PRODUCT       "MT65xx MS"
#define USB_MS_RELEASE       0x0100

#define CHR_TYPE_DETECT_DEB  400  /* debounce time for charger type detection, in ms */

#endif /* MTK_USB_CUSTOM_H */
